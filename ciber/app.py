from flask import Flask, request, jsonify, render_template
from datetime import datetime

app = Flask(__name__)

# Armazena dados recebidos e comando atual
dados_esp32 = []
comando_atual = {"acao": "auto"}

@app.route('/api/dados', methods=['POST'])
def receber_dados():
    global dados_esp32
    content = request.json
    if content is None or "temperatura" not in content or "umidade" not in content:
        return jsonify({"status": "erro", "mensagem": "Dados inválidos"}), 400

    dado = {
        "temperatura": content["temperatura"],
        "umidade": content["umidade"],
        "hora": datetime.now().strftime('%d/%m/%Y %H:%M:%S')
    }

    dados_esp32.append(dado)

    print(f"📥 Recebido dados: {dado}")
    return jsonify({"status": "ok"})

@app.route('/api/dados', methods=['GET'])
def enviar_ultimo_dado():
    if dados_esp32:
        return jsonify(dados_esp32[-1])
    else:
        return jsonify({})

@app.route('/api/comando', methods=['GET', 'POST'])
def comando():
    global comando_atual

    if request.method == 'POST':
        content = request.json
        if content and "acao" in content:
            comando_atual["acao"] = content["acao"]
            print(f"🛠️ Comando atualizado para: {comando_atual['acao']}")
            return jsonify({"status": "ok", "acao": comando_atual["acao"]})

        return jsonify({"status": "erro", "mensagem": "JSON inválido"}), 400

    return jsonify(comando_atual)

@app.route('/')
def index():
    return render_template('index.html', dados=reversed(dados_esp32), comando=comando_atual["acao"])

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
