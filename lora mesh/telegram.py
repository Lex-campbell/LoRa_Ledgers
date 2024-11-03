import requests
from flask import Flask, request, jsonify

app = Flask(__name__)

BOT_TOKEN = "7520876386:AAGi1FeV9XC6wdHZyc6EVpvuRW7DPHZBgjU"
TELEGRAM_API = f"https://api.telegram.org/bot{BOT_TOKEN}"
CHAT_ID = "795879280" # Replace with your chat ID

@app.route('/send_message', methods=['POST'])
def send_message():
    try:
        data = request.get_json()
        message = data.get('message')
        
        if not message:
            return jsonify({'error': 'No message provided'}), 400

        # Send message to Telegram
        payload = {
            'chat_id': CHAT_ID,
            'text': message,
            'parse_mode': 'HTML'
        }
        
        response = requests.post(f"{TELEGRAM_API}/sendMessage", json=payload)
        
        if response.status_code == 200:
            return jsonify({'success': True, 'message': 'Message sent successfully'}), 200
        else:
            return jsonify({'error': f'Failed to send message. Status code: {response.status_code}'}), 500

    except Exception as e:
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=6969)
