import cv2
import math
from ultralytics import YOLO
import cvzone

# Load YOLO model for fire and smoke detection
model = YOLO('Hoafinale.pt')

# Classes (assuming fire and smoke are the classes)
classnames = ['fire', 'smoke']

# Function to send notification (replace with your preferred method)
def send_fire_notification():
    # Example methods (choose one or implement your own):
    # - Email notification
    # import smtplib
    # email = 'your_email@example.com'
    # password = 'your_email_password'
    # message = 'Fire detected!'
    # server = smtplib.SMTP('smtp.example.com', 587)
    # server.starttls()
    # server.login(email, password)
    # server.sendmail(email, email, message)
    # server.quit()

    # - SMS notification (using a service like Twilio)
    # import requests
    # account_sid = 'your_account_sid'
    # auth_token = 'your_auth_token'
    # from_number = '+1your_twilio_number'
    # to_number = '+1your_phone_number'
    # message = 'Fire detected!'
    # url = f'https://api.twilio.com/Accounts/{account_sid}/Messages.json'
    # data = {
    #     'From': from_number,
    #     'To': to_number,
    #     'Body': message
    # }
    # headers = {'Authorization': f'Basic {base64.b64encode(f'{account_sid}:{auth_token}'.encode()).decode()}'}
    # requests.post(url, headers=headers, data=data)

    # - Push notification (using a service like Firebase Cloud Messaging)
    # (Implementation depends on your chosen service)

    print("Fire or smoke detected! Sending notification...")

# Video capture (replace 'test-chay.webp' with your video source)
cap = cv2.VideoCapture('smoke2.jpg')

while True:
    ret, frame = cap.read()
    if not ret:
        break

    # Resize frame for efficiency
    frame = cv2.resize(frame, (640, 480))

    # Fire and smoke detection using YOLO
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
                color = (0, 0, 255) if class_name == 1 else (255, 0, 0)  # Red for fire, blue for smoke
                cv2.rectangle(frame, (x1, y1), (x2, y2), color, 5)
                cvzone.putTextRect(frame, f'{classnames[class_name]} {confidence}%', [x1 + 8, y1 + 100],
                                  scale=1.5, thickness=2)

                # Send notification on fire or smoke detection
                send_fire_notification()

    cv2.imshow('Fire and Smoke Detection', frame)

    # Quit on 'q' key press
    if cv2.waitKey(0) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
