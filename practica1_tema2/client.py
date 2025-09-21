import socket

PORT = 8082
HOST = '127.0.0.1'

def main():
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client:
        client.connect((HOST, PORT))
        # pedimos credenciales
        user = input('username: ')
        pwd = input('password: ')
        # enviamos al credenciales para validar
        client.send((user + '|' + pwd).encode('utf-8'))

        while True: # para esuchar las mensajes del servidor
            data = client.recv(1024)
            if not data:
                print("Connection closed")
                break
            message = data.decode('utf-8')

            print(f'message from server: {message}')

            if message == "EXIT":
                client.close()
                break

            # pedimos mensajes al cliente
            messageToSend = input('your message: ')
            client.send(messageToSend.encode('utf-8'))



if __name__ == '__main__':
    main()