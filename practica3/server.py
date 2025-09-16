import socket
import threading
from os import environ

def handle_connection(conn, addr):
    print("Connection from", addr)
    while conn: # mientras la conexion siga viva
        credencialesBin = conn.recv(1024) # recibimos credenciales del usuario
        if not credencialesBin:
            print("Connection closed")
            break
        credentials = credencialesBin.decode('utf-8')
        # explotamos la cadena para valudar credenciales
        credentialParts = credentials.split('|')
        if len(credentialParts) != 2:
            #indicamos que que la contraseña debe estar compuesta por usuario y contraeña
            conn.sendall(b'credenciales mal formadas')
            conn.close()
            break
        user, passwd = credentialParts

        if passwd != environ.get('PASSWD'):
            conn.sendall(b'credenciales no validas')
            conn.close()
            break

        conn.sendall(b'conexion establecida')

        while True: # bucle infinito para escuchar la informacion entrante
            data = conn.recv(1024)
            if not data: # si no envio datos, se desconecto
                print("Connection closed")
                break
            message = data.decode('utf-8')
            print(f'message from {user}: {message}')
            # mandamos un echo
            conn.sendall(message.encode('utf-8'))
    pass


def main():
    port = environ.get('PORT') or '8080'
    host = environ.get('HOST') or '0.0.0.0'
    port = int(port)

    # creamos un socket bajo TCP
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.bind((host, port))

        print(f"Listening on {host}:{port}")

        server.listen() # iniciamos la servidor para escuchar conexiones


        while True:
            # mandamos a handlear la conexion a otro hilo para recibir mas conexiones
            conn, addr = server.accept()
            # mandamos la conexion a otro hilo para que maneje su propia conexion
            thread = threading.Thread(target=handle_connection, args=(conn, addr))
            thread.start() # iniciamos le ejecucion del hilo



if __name__ == '__main__':
    main()