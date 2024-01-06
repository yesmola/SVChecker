from flask import Flask, request, jsonify
from flask_cors import CORS

from test import MyService
from test.ttypes import *
from thrift.transport import TSocket
from thrift.transport import TTransport
from thrift.protocol import TBinaryProtocol

# transport = TSocket.TSocket('localhost', 9090)
# transport = TTransport.TBufferedTransport(transport)
# protocol = TBinaryProtocol.TBinaryProtocol(transport)
# client = MyService.Client(protocol)
# transport.open()
    
app = Flask(__name__)
CORS(app)

@app.route('/upload', methods=['POST'])
def handle_file_upload():
    file = request.files['file']
    file.save('./test.sol')
    return jsonify({'message': 'File uploaded successfully!'})

if __name__ == '__main__':
    # res = client.myFunction('fff')
    # print(res)
    
    app.run()
