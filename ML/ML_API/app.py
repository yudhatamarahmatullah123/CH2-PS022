from flask import Flask, jsonify
import pickle
import pandas as pd
import mysql.connector
import paho.mqtt.publish as publish
import json
import os

app = Flask(__name__)
#logging.basicConfig(level=logging.DEBUG)

latest_data = pd.DataFrame()

try:
    cluster_model = pickle.load(open('cluster_model.pkl', 'rb'))
    rules = pickle.load(open('rules_model.pkl', 'rb'))
except Exception as e:
    logging.error(f"Error loading models: {e}")
    raise e

db_config = {
    'host': '34.31.59.23',
    'user': 'pale',
    'password': 'Pale123!',
    'database': 'pale_db'
}
def fetch_and_process_data():
    global latest_data
    try:
        connection = mysql.connector.connect(**db_config)
        if connection.is_connected():
            cursor = connection.cursor(dictionary=True)
            # Query SQL Anda
            query = "SELECT * FROM pale_db.pale_tables;"

            cursor.execute(query)
            data = cursor.fetchall()
            cursor.close()
            df = pd.DataFrame(data)
            # slice data
            df = df[df.columns[2:5]]
            print(df)
            df.columns = ['Suhu', 'pH', 'Amonia']
            df = apply_thresholds(df)
            df = cluster(df, cluster_model)
            df['Binary Pattern'], df['Readable Pattern'] = zip(*df.apply(get_pattern, axis=1))

            # Perbarui variabel global dengan data yang sudah diproses terbaru
            latest_data = df.copy()
    except mysql.connector.Error as err:
        logging.error(f'Error fetching data: {err}')

@app.route('/predict', methods=['GET', 'POST'])
def predict():
    fetch_and_process_data()
    try:
        global latest_data
        df = latest_data.copy()
        pattern = []
        warning = []

        for i in range(len(df)):
            data = df['Binary Pattern'][i]
            if data in rules:
                pattern.append(data)
                warning.append(int(1))
            else:
                warning.append(int(0))
        for i in warning:
            if i == 1:
               send_mqtt_message('on')
            else:
                send_mqtt_message('off')
        return jsonify({"Status": warning})

    except Exception as e:
        logging.error(f"Error processing request: {e}")
        return jsonify({"error": "Internal Server Error"}), 500

def apply_thresholds(df):
    column_thresholds = {
        'Suhu': (20.0, 30.0),
        'pH': (6.0, 9.0),
        'Amonia': (0.0, 0.8)
    }

    for column, thresholds in column_thresholds.items():
        lower_bound, upper_bound = thresholds

        # Convert the column to numeric values
        df[column] = pd.to_numeric(df[column], errors='coerce')

        # Apply thresholds
        df[f'{column.lower()}_thresholded'] = df[column].apply(lambda val: 1 if (val is not None) and (val > upper_bound or val < lower_bound) else 0)
    return df
def get_pattern(raw_features):
    pattern_binary = ''
    pattern_readable = ''

    for i in range(3):
        feature_name = raw_features.index[i]
        is_failure = raw_features.iloc[i + 3] == 1
        pattern_readable += f'{feature_name}: {"Failure" if is_failure else "Normal"}, '
        pattern_binary += str(int(raw_features.iloc[i + 3]))

    return pattern_binary, pattern_readable

def cluster(df,loaded_model):
    df['Condition'] = loaded_model.predict(df[df.columns[3:7]])
    return df

def send_mqtt_message(warning):
    #payload = json.dumps(warning)
    publish.single("katup", warning, hostname="34.31.59.23", port=1883, auth={
        'username': 'pale',
        'password': 'Pale123!'
    })



if __name__ == '__main__':
	app.run(host="0.0.0.0", port=int(os.environ.get('PORT', 8080)))
