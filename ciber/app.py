from flask import Flask, request, jsonify, render_template
from datetime import datetime

app = Flask(__name__)

dados = []

 #API Recebe dados do ESP32 
@app.route('/api/dados', methods=['POST'])
def receber_dados():
    content = request.json
    temperatura = content.get('temperatura')
    umidade = content.get('umidade')
    timestamp = datetime.now().strftime('%d/%m/%Y %H:%M:%S')

    dados.append({
        'temperatura': temperatura,
        'umidade': umidade,
        'hora': timestamp
    })

    print(f"Recebido: {temperatura}°C | {umidade}% às {timestamp}")
    return jsonify({'status': 'OK'})

#API Envia o último dado 

@app.route('/api/dados', methods=['GET'])
def enviar_dados():
    if dados:
        return jsonify(dados[-1])
    else:
        return jsonify({'hora': '', 'temperatura': '', 'umidade': ''})

#Página Principal
@app.route('/')
def index():
    return render_template('index.html', dados=reversed(dados))

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5000, debug=True)