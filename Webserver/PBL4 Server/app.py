from flask import Flask, jsonify, render_template, url_for
from flask_mysqldb import MySQL
import os

app = Flask(__name__)

# Cấu hình MySQL
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
        # Lấy đường dẫn hình ảnh từ cơ sở dữ liệu
        cur = mysql.connection.cursor()
        cur.execute("SELECT image_path FROM detected_GPS ORDER BY ID DESC LIMIT 1")
        image_data = cur.fetchone()
        cur.close()

        if image_data:
            image_path = image_data[0]  # Lấy đường dẫn hình ảnh
            full_image_path = os.path.join('AI_pycode', image_path).replace('\\', '/')
            print(f"Full image path: {full_image_path}")  # In ra để kiểm tra
        else:
            full_image_path = ''  # Đường dẫn hình ảnh mặc định

    except Exception as e:
        print(f"Error: {e}")
        full_image_path = ''  # Đường dẫn hình ảnh mặc định

    return render_template('identification.html', image_path=full_image_path)

@app.route('/identification/<path:image_path>')
def identification(image_path):
    try:
        if image_path:
            full_image_path = os.path.join('AI_pycode', image_path).replace('\\', '/')        
            print(f"Image path: {full_image_path}")
        else:
            full_image_path = '' 
        
    except Exception as e:
        print(f"Error: {e}")
        full_image_path = ''  # Đường dẫn hình ảnh mặc định

    return render_template('identification.html', image_path=full_image_path)

@app.route('/api/get_latest_coordinates')
def get_latest_coordinates():
    try:
        cur = mysql.connection.cursor()
        cur.execute("SELECT latitude, longitude FROM detected_GPS ORDER BY ID DESC LIMIT 1")
        result = cur.fetchone()
        cur.close()

        if result:
            return jsonify({
                "latitude": result[0],
                "longitude": result[1]
            })
        else:
            return jsonify({
                "error": "No coordinates found"
            }), 404
    except Exception as e:
        print(f"Error: {e}")
        return jsonify({
            "error": "Failed to fetch coordinates"
        }), 500

# @app.route('/analysis')
# def analysis():
#     return render_template('analysis.html')

# @app.route('/storage')
# def storage():
#     try:
#         cur = mysql.connection.cursor()
#         cur.execute("SELECT ID, Datetime, image_path FROM detected ORDER BY ID DESC LIMIT 10")
#         images_data = cur.fetchall() 
#         cur.close()

#         image_info = []
#         for record in images_data:
#             image_info.append({
#                 'id': record[0],
#                 'datetime': record[1],
#                 'image_path': record[2],
#                 'coordinates': (0, 0)  # Thay thế bằng tọa độ thực tế nếu có
#             })

#     except Exception as e:
#         print(f"Error: {e}")
#         image_info = []  

#     return render_template('storage.html', image_info=image_info)


@app.route('/storage/<int:page>')
@app.route('/storage/')
def storage(page=1):
    try:
        cur = mysql.connection.cursor()
        offset = (page - 1) * 10  # Calculate the offset for pagination
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
                'coordinates': (record[3], record[4])  # Use actual latitude and longitude
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


@app.route('/api/get_storage_data')
def get_storage_data():
    try:
        page = request.args.get('page', default=1, type=int)
        offset = (page - 1) * 10

        cur = mysql.connection.cursor()
        cur.execute("""
            SELECT ID, Datetime, image_path, latitude, longitude
            FROM detected_GPS
            ORDER BY ID DESC
            LIMIT 10 OFFSET %s
        """, (offset,))
        records = cur.fetchall()
        cur.close()

        image_info = [
            {
                "id": record[0],
                "datetime": record[1],
                "image_path": record[2],
                "coordinates": [record[3], record[4]],
            }
            for record in records
        ]

        cur = mysql.connection.cursor()
        cur.execute("SELECT COUNT(*) FROM detected_GPS")
        total_records = cur.fetchone()[0]
        cur.close()

        total_pages = (total_records // 10) + (1 if total_records % 10 > 0 else 0)

        return jsonify({
            "image_info": image_info,
            "page": page,
            "total_pages": total_pages,
        })

    except Exception as e:
        print(f"Error: {e}")
        return jsonify({
            "error": "Failed to fetch storage data"
        }), 500

if __name__ == "__main__":
    app.run(debug=True)