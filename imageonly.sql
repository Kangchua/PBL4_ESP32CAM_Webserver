-- Create the database
CREATE DATABASE DRONE;

-- Use the newly created database
USE DRONE;

-- Create the table 'detected'
CREATE TABLE detected (
    ID INT AUTO_INCREMENT PRIMARY KEY,
    Datetime DATETIME NOT NULL,
    image_path VARCHAR(255) NOT NULL
);

SELECT * From detected;
SELECT Datetime, image_path FROM detected ORDER BY ID DESC LIMIT 1;
drop database DRONE