DROP DATABASE IF EXISTS LibGuardian;
CREATE DATABASE LibGuardian;
USE LibGuardian;

-- Table to store rooms with unique names
CREATE TABLE IF NOT EXISTS rooms (
    room_id INT AUTO_INCREMENT PRIMARY KEY,
    room_name VARCHAR(100) NOT NULL UNIQUE
    -- room_capacity INT NOT NULL
);

-- Base sensors table, each sensor linked to a room (nullable)
CREATE TABLE IF NOT EXISTS sensors (
    sensor_id INT AUTO_INCREMENT PRIMARY KEY,
    description VARCHAR(100),
    -- location VARCHAR(50),
    room_id INT,
    FOREIGN KEY (room_id) REFERENCES rooms(room_id) ON DELETE SET NULL
);

-- Noise sensors inherit from sensors, with specific noise model info
CREATE TABLE IF NOT EXISTS noise_sensors (
    noise_sensor_id INT AUTO_INCREMENT PRIMARY KEY,
    sensor_id INT NOT NULL,
    noise_model VARCHAR(50),
    FOREIGN KEY (sensor_id) REFERENCES sensors(sensor_id) ON DELETE CASCADE
);

-- Proximity sensors inherit from sensors, with specific proximity model info
CREATE TABLE IF NOT EXISTS proximity_sensors (
    proximity_sensor_id INT AUTO_INCREMENT PRIMARY KEY,
    sensor_id INT NOT NULL,
    proximity_model VARCHAR(50),
    FOREIGN KEY (sensor_id) REFERENCES sensors(sensor_id) ON DELETE CASCADE
);

-- Association of doors to two sensors (entry and exit), linked to a room
CREATE TABLE IF NOT EXISTS door_sensors ( 
    door_id INT PRIMARY KEY NOT NULL,
    entry_sensor_id INT NOT NULL,
    exit_sensor_id INT NOT NULL,
    door_name VARCHAR(100) NOT NULL,
    room_id INT NOT NULL,
    FOREIGN KEY (entry_sensor_id) REFERENCES sensors(sensor_id) ON DELETE CASCADE,
    FOREIGN KEY (exit_sensor_id) REFERENCES sensors(sensor_id) ON DELETE CASCADE,
    FOREIGN KEY (room_id) REFERENCES rooms(room_id) ON DELETE CASCADE
);

-- Raw sensor event data, timestamped, for all sensors
CREATE TABLE IF NOT EXISTS sensor_events (
    id INT AUTO_INCREMENT PRIMARY KEY,
    sensor_id INT NOT NULL,
    readings FLOAT,
    timestamp DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (sensor_id) REFERENCES sensors(sensor_id) ON DELETE CASCADE
);

-- People movements detected by door sensors (entrances/exits), count updated per event
CREATE TABLE IF NOT EXISTS people_movements (
    id INT AUTO_INCREMENT PRIMARY KEY,
    door_id INT NOT NULL,
    timestamp DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    movement_type ENUM('entrada', 'salida') NOT NULL,
    number_of_people INT NOT NULL DEFAULT 1,
    FOREIGN KEY (door_id) REFERENCES door_sensors(door_id) ON DELETE CASCADE
);

-- Separate tables for proximity and sound readings indexed by sensor and timestamp for efficient querying
CREATE TABLE IF NOT EXISTS proximity_readings (
    id INT AUTO_INCREMENT PRIMARY KEY,
    sensor_id INT NOT NULL,
    timestamp DATETIME NOT NULL,
    INDEX(sensor_id),
    INDEX(timestamp)
);

CREATE TABLE IF NOT EXISTS sound_readings (
    id INT AUTO_INCREMENT PRIMARY KEY,
    sensor_id INT NOT NULL,
    timestamp DATETIME NOT NULL,
    INDEX(sensor_id),
    INDEX(timestamp)
);

-- Trigger to split raw sensor events into proximity or sound readings, based on sensor type
DELIMITER $$
CREATE TRIGGER trg_split_sensor_events
AFTER INSERT ON sensor_events
FOR EACH ROW
BEGIN
    IF EXISTS (SELECT 1 FROM proximity_sensors WHERE sensor_id = NEW.sensor_id) THEN
        INSERT INTO proximity_readings (sensor_id, timestamp)
        VALUES (NEW.sensor_id, NEW.timestamp);
    ELSEIF EXISTS (SELECT 1 FROM noise_sensors WHERE sensor_id = NEW.sensor_id) THEN
        INSERT INTO sound_readings (sensor_id, timestamp)
        VALUES (NEW.sensor_id, NEW.timestamp);
    END IF;
END$$
DELIMITER ;

-- Trigger to detect people movement by analyzing pairs of proximity sensor readings at doors
DELIMITER $$
CREATE TRIGGER trg_add_people_movement
AFTER INSERT ON proximity_readings
FOR EACH ROW
BEGIN
    DECLARE paired_sensor INT;
    DECLARE is_entry TINYINT(1);
    DECLARE doorId INT;
    DECLARE last_count INT DEFAULT 0;
    DECLARE last_opposite_time DATETIME;

    -- Find the door and the paired sensor for the current sensor reading
    SELECT ds.door_id,
           CASE 
               WHEN ds.entry_sensor_id = NEW.sensor_id THEN ds.exit_sensor_id
               ELSE ds.entry_sensor_id
           END AS paired,
           CASE
               WHEN ds.entry_sensor_id = NEW.sensor_id THEN 1
               ELSE 0
           END AS entry_flag
    INTO doorId, paired_sensor, is_entry
    FROM door_sensors ds
    WHERE ds.entry_sensor_id = NEW.sensor_id OR ds.exit_sensor_id = NEW.sensor_id
    LIMIT 1;

    -- If a door is found
    IF doorId IS NOT NULL THEN
        -- Find the last reading from the paired sensor within 5 seconds before the current reading
        SELECT timestamp
        INTO last_opposite_time
        FROM proximity_readings
        WHERE sensor_id = paired_sensor
          AND timestamp <= NEW.timestamp
          AND TIMESTAMPDIFF(SECOND, timestamp, NEW.timestamp) <= 5
        ORDER BY timestamp DESC
        LIMIT 1;

        -- If there is a valid paired sensor reading
        IF last_opposite_time IS NOT NULL THEN
            -- Get the last known number of people for this door
            SELECT number_of_people
            INTO last_count
            FROM people_movements
            WHERE door_id = doorId
            ORDER BY timestamp DESC
            LIMIT 1;

            -- Calculate new people count:
            -- If current sensor is entry sensor, count decreases (someone left)
            -- Else count increases (someone entered)
            SET last_count = GREATEST(0, last_count + IF(is_entry = 1, -1, 1));

            -- Insert new movement record with updated count and movement type
            INSERT INTO people_movements (door_id, movement_type, number_of_people, timestamp)
            VALUES (
                doorId,
                IF(is_entry = 1, 'salida', 'entrada'),
                last_count,
                NEW.timestamp
            );
        END IF;
    END IF;
END$$
DELIMITER ;

-- Sample data inserts for rooms, sensors, noise sensors, proximity sensors, and doors

INSERT INTO rooms (room_name) VALUES
('Room 101'),            -- id 1
('Room 102');            -- id 2

INSERT INTO sensors (description, room_id) VALUES
('Entry Room 1', 1),                -- id 1
('Exit Room 1', 1),                 -- id 2
('Noise Room 1', 1),                -- id 3

('Entry Room 2', 2),                -- id 4
('Exit Room 2', 2),                 -- id 5
('Noise Room 2', 2);                -- id 6


INSERT INTO noise_sensors (sensor_id, noise_model) VALUES
(3, 'MicModel X'),
(6, 'MicModel X');

INSERT INTO proximity_sensors (sensor_id, proximity_model) VALUES
(1, 'PIR-X'),
(2, 'Prox-A'),

(4, 'Prox-B'),
(5, 'Prox-B');

INSERT INTO door_sensors (door_id, entry_sensor_id, exit_sensor_id, door_name, room_id) VALUES
(1, 1, 2, 'Door Room 101', 1),
(2, 4, 5, 'Door Room 102', 2);

-- Example sensor_events inserts commented out:
-- INSERT INTO sensor_events (sensor_id) VALUES (3),(4);

-- Example manual people movement insert commented out:
-- INSERT INTO people_movements (door_id, movement_type, number_of_people) VALUES(1, 'entrada', 0);