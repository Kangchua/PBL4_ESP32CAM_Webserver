import cv2
import urllib.request
import numpy as np
import math
from ultralytics import YOLO
import cvzone
import os
from datetime import datetime
import time

# Load YOLO model for fire detection
model = YOLO('mybest.pt')

# Classes (assuming fire is the only class)
classnames = ['smoke', 'fire']

# Function to send notification (replace with your preferred method)
def send_fire_notification():
    print("Fire detected! Sending notification...")

# URL for the capture image
url = 'http://192.168.110.90/capture'

# Create directory for saving images if it doesn't exist
if not os.path.exists('imagescan'):
    os.makedirs('imagescan')

cv2.namedWindow("Live Transmission", cv2.WINDOW_AUTOSIZE)

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

                    # Send notification on fire detection
                    send_fire_notification()

                    # Save the frame with detected fire
                    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
                    filename = os.path.join('imagescan', f'fire_{timestamp}.jpg')
                    cv2.imwrite(filename, frame)
                    print(f'Image saved: {filename}')

        cv2.imshow("Live Transmission", frame)

        # Quit on 'q' key press
        if cv2.waitKey(5) & 0xFF == ord('q'):
            break

        # Delay to ensure only one image is processed at a time
        time.sleep(5)  # Adjust the delay as needed

    except Exception as e:
        print(f"Error: {e}")

cv2.destroyAllWindows()
