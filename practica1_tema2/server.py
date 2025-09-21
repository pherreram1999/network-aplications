import json
import socket
import threading
import re
import secrets
from os import environ

from database import DatabasePool

PORT = '8082'

def openDB():
    with open('connections.json') as connectionsJSON:
        connections = json.load(connectionsJSON)
        return DatabasePool(connections)

def handle_connection(conn, addr,db: DatabasePool):
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

            # si el mensaje es un DEL vamos a explorar para eliminar
            if re.match(r'^DEL .*',message):
                # quiere eliminar un mensaje
                clave = message.strip('DEL ')
                # buscamos en base al valor eliminar
                try:
                    db.deleteMessage(clave)
                    conn.sendall(b'mensaje eliminado')
                except Exception as e:
                    print('excepetion on delete:',e)
                    conn.sendall(b'no fue posible el mensaje')
                continue

            # si el mensaje es un un comando list
            if message == 'LIST':
                # enviamos al usuarios los ultimos mensajes desde base
                last_messages = db.getLastMessages()
                conn.sendall(last_messages)
                continue

            print(f'message from {user}: {message}')
            # registramos el mensaje pool de base datos

            message_id = secrets.token_urlsafe(8)

            db.execute(
                'INSERT INTO messages (uuid,client, message) VALUES (?, ?,?)',
                (message_id,user, message)
            )
            # mandamos un echo
            conn.sendall(message.encode('utf-8'))
    pass


def main():
    port = environ.get('PORT') or PORT
    host = environ.get('HOST') or '0.0.0.0'
    port = int(port)

    dbconn = openDB()
    dbconn.migrate() # aseguramos que exitan las tablas

    # creamos un socket bajo TCP
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server:
        server.bind((host, port))

        print(f"Listening on {host}:{port}")

        server.listen() # iniciamos la servidor para escuchar conexiones
        while True:
            # mandamos a handlear la conexion a otro hilo para recibir mas conexiones
            conn, addr = server.accept()
            # mandamos la conexion a otro hilo para que maneje su propia conexion
            thread = threading.Thread(target=handle_connection, args=(conn, addr, dbconn))
            thread.start() # iniciamos le ejecucion del hilo



if __name__ == '__main__':
    main()