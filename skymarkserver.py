from flask import Flask, request, jsonify
import mysql.connector

app = Flask(__name__)

gate_mapping = {
    "entry_gate1": ("entryboom", 1),
    "entry_gate2": ("entryboom", 2),
    "exit_gate1": ("exitboom", 1),
    "exit_gate2": ("exitboom", 2),
}

def get_db_connection():
    return mysql.connector.connect(host='localhost', user='root', password='', database='skymark')

@app.route('/check_boomsig', methods=['GET'])
def check_boom_signal():
    gate_key = request.args.get('gate_id')
    boom_column_and_id = gate_mapping.get(gate_key)

    if boom_column_and_id is None:
        return jsonify({"error": "Invalid gate_id"}), 400

    column_name, gate_id = boom_column_and_id

    try:
        conn = get_db_connection()
        cursor = conn.cursor()

        query = f'SELECT {column_name} FROM boomsig WHERE id = %s'
        cursor.execute(query, (gate_id,))
        row = cursor.fetchone()
        print(f"Fetched Row for Gate {gate_id}: {row}")

        if row and row[0] == 'Y':
            update_query = f'UPDATE boomsig SET {column_name} = "N" WHERE id = %s'
            cursor.execute(update_query, (gate_id,))
            conn.commit()
            conn.close()
            return jsonify({"command": "|OPENEN%"}), 200

        conn.close()
        return jsonify({"command": "NO_ACTION"}), 200

    except mysql.connector.Error as err:
        print(f"MySQL Error: {err}")
        return jsonify({"error": f"MySQL Error: {err}"}), 500

if __name__ == '__main__':
    app.run(host='192.168.3.100', port=5000, debug=True, threaded=True)
