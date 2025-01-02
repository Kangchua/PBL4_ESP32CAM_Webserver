from flask import Flask, jsonify, render_template, request, url_for
from flask_mysqldb import MySQL
import os

app = Flask(__name__)

# MySQL Configuration
app.config['MYSQL_HOST'] = '127.0.0.1'
app.config['MYSQL_USER'] = 'root'
app.config['MYSQL_PASSWORD'] = '123456'
app.config['MYSQL_DB'] = 'DRONE'

mysql = MySQL(app)

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/header')
def header():
    return render_template('header.html')

@app.route('/left_content')
def left_content():
    return render_template('left-content.html')

@app.route('/identification')
def identification_default():
    try:
        cur = mysql.connection.cursor()
        cur.execute("""
            SELECT image_path, latitude, longitude 
            FROM detected_GPS 
            ORDER BY ID DESC 
            LIMIT 1
        """)
        latest_data = cur.fetchone()
        cur.close()

        if latest_data:
            image_path, latitude, longitude = latest_data
            full_image_path = os.path.join('AI_pycode', image_path).replace('\\', '/')
            coordinates = (latitude, longitude)
        else:
            full_image_path = ''
            coordinates = (0, 0)  # Default coordinates if no data is found

    except Exception as e:
        print(f"Error: {e}")
        full_image_path = ''
        coordinates = (0, 0)

    return render_template('identification.html', image_path=full_image_path, coordinates=coordinates)

@app.route('/identification/<path:image_path>')
def identification(image_path):
    try:
        latitude = request.args.get('latitude', default=None, type=float)
        longitude = request.args.get('longitude', default=None, type=float)

        full_image_path = os.path.join('AI_pycode', image_path).replace('\\', '/') if image_path else ''

        if latitude is not None and longitude is not None:
            coordinates = (latitude, longitude)
        else:
            coordinates = (0, 0)

        print(f"Image path: {full_image_path}, Coordinates: {coordinates}")

    except Exception as e:
        print(f"Error: {e}")
        full_image_path = ''
        coordinates = (0, 0)

    return render_template('identification.html', image_path=full_image_path, coordinates=coordinates)


@app.route('/storage/')
@app.route('/storage/<int:page>')
def storage(page=1):
    try:
        # Calculate the offset for pagination
        offset = (page - 1) * 10  
        cur = mysql.connection.cursor()
        cur.execute("""
            SELECT ID, Datetime, image_path, latitude, longitude
            FROM detected_GPS
            ORDER BY ID DESC
            LIMIT 10 OFFSET %s
        """, (offset,))
        images_data = cur.fetchall()
        cur.close()

        image_info = [
            {
                'id': record[0],
                'datetime': record[1],
                'image_path': record[2],
                'coordinates': (record[3], record[4])
            }
            for record in images_data
        ]

        # Get the total number of records for pagination
        cur = mysql.connection.cursor()
        cur.execute("SELECT COUNT(*) FROM detected_GPS")
        total_records = cur.fetchone()[0]
        cur.close()

        total_pages = (total_records // 10) + (1 if total_records % 10 > 0 else 0)

    except Exception as e:
        print(f"Error: {e}")
        image_info = []
        total_pages = 1  # Default to 1 page if there is an error

    return render_template('storage.html', image_info=image_info, page=page, total_pages=total_pages)

if __name__ == "__main__":
    app.run(debug=True)
