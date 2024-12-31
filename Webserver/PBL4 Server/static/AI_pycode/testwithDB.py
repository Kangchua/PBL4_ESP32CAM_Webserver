import mysql.connector
import cv2
import urllib.request
import numpy as np
import math
from ultralytics import YOLO
import cvzone
import os
from datetime import datetime
import time
from http.server import BaseHTTPRequestHandler, HTTPServer

# Connect to MySQL database
mydb = mysql.connector.connect(
    host="localhost",
    user="root",
    password="123456",
    database="DRONE"
)

mycursor = mydb.cursor()

# Function to save image info to the database
def save_image_info(image_path):
    timestamp = datetime.now()
    sql_check = "SELECT * FROM detected WHERE Datetime = %s"
    val_check = (timestamp,)
    mycursor.execute(sql_check, val_check)
    result = mycursor.fetchone()
    
    if result is None:
        sql_insert = "INSERT INTO detected (Datetime, image_path) VALUES (%s, %s)"
        val_insert = (timestamp, image_path)
        mycursor.execute(sql_insert, val_insert)
        mydb.commit()
        print(f"Image info saved to database: {timestamp}, {image_path}")
    else:
        print(f"Duplicate image detected: {timestamp}, {image_path}")

# Load YOLO model for fire detection
model = YOLO('Hoa best.pt')

# Classes (assuming fire is the only class)
classnames = ['fire', 'smoke']

# Function to send notification (replace with your preferred method)
def send_fire_notification():
    print("Fire detected! Sending notification...")

# URL for the capture image
url = 'http://172.20.10.2/capture'

# Create directory for saving images if it doesn't exist
if not os.path.exists('imagescan'):
    os.makedirs('imagescan')

cv2.namedWindow("Live Transmission", cv2.WINDOW_AUTOSIZE)

image_saved = False

while True:
    try:
        # Fetch the image from the URL
        img_resp = urllib.request.urlopen(url)
        imgnp = np.array(bytearray(img_resp.read()), dtype=np.uint8)
        img = cv2.imdecode(imgnp, -1)

        # Resize frame for efficiency
        frame = cv2.resize(img, (1024, 768))

        # Fire detection using YOLO
        result = model(frame, stream=True)

        # Process results and draw bounding boxes
        for info in result:
            boxes = info.boxes
            for box in boxes:
                confidence = box.conf[0]
                confidence = math.ceil(confidence * 100)
                class_name = int(box.cls[0])
                if confidence > 50:
                    x1, y1, x2, y2 = box.xyxy[0]
                    x1, y1, x2, y2 = int(x1), int(y1), int(x2), int(y2)
                    cv2.rectangle(frame, (x1, y1), (x2, y2), (0, 0, 255), 5)
                    cvzone.putTextRect(frame, f'{classnames[class_name]} {confidence}%', [x1 + 8, y1 + 100],
                                      scale=1.5, thickness=2)

                    if not image_saved:
                        # Send notification on fire detection
                        send_fire_notification()

                        # Save the frame with detected fire
                        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
                        filename = os.path.join('imagescan', f'fire_{timestamp}.jpg')
                        cv2.imwrite(filename, frame)
                        print(f'Image saved: {filename}')

                        # Save image info to database
                        save_image_info(filename)

                        # Set the flag to indicate the image has been saved
                        image_saved = True

        cv2.imshow("Live Transmission", frame)

        # Quit on 'q' key press
        if cv2.waitKey(5) & 0xFF == ord('q'):
            break

        # Reset the flag after a delay to allow for new detections
        time.sleep(5)  # Adjust the delay as needed
        image_saved = False

    except Exception as e:
        print(f"Error: {e}")

cv2.destroyAllWindows()