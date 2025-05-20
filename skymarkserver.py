from flask import Flask, request, jsonify
import mysql.connector
app = Flask(__name__)

gate_mapping = {
    "entry_gate1": 1,
    "entry_gate2": 2,
    "exit_gate1": 3,
    "exit_gate2": 4,
}

def get_db_connection():
    conn = mysql.connector.connect(host='localhost', user='root', password='', database='vcd')
    return conn

@app.route('/check_boomsig', methods=['GET'])
def check_boom_signal():
    gate_key = request.args.get('gate_id')
    gate_id = gate_mapping.get(gate_key)


    if gate_id is None:
        return jsonify({"error": "Invalid gate_id"}), 400

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        cursor.execute('SELECT entryboom FROM boomsig WHERE id = %s', (gate_id,))
        row = cursor.fetchone()
        print(f"Fetched Row for Gate {gate_id}: {row}")  

        if row and row[0] == 'Y':          
            cursor.execute('UPDATE boomsig SET entryboom = "N" WHERE id = %s', (gate_id,))
            conn.commit()
            conn.close()

            return jsonify({"command": "|OPENEN%"}), 200

        conn.close()
        return jsonify({"command": "NO_ACTION"}), 200

    except mysql.connector.Error as err:
        print(f"MySQL Error: {err}")  
        return jsonify({"error": f"MySQL Error: {err}"}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True, threaded=True)
