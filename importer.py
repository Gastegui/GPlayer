import atexit
import re
import sys
import os
import yt_dlp
import mysql.connector
from mysql.connector import Error
from mysql.connector import MySQLConnection
from mysql.connector.cursor import MySQLCursor
from typing import Optional

connection = None
cursor = None


def exit():
    if cursor:
        cursor.close()
    if connection and connection.is_connected():
        connection.close()
    print("Cerrado correctamente")

def connect():
    global connection
    global cursor

    try:
        connection = mysql.connector.connect(
            host="localhost",
            database="GPlayer",
            user="root",
            password="MariaDB_P4ssw0rd-123",
            port=3306,
            autocommit = True
        )
        
        cursor = connection.cursor()

        atexit.register(exit)
        
    except Error as e:
        print(f"Error: {e}")

def conseguirArtistas() -> list:
    cursor.execute("SELECT Nombre FROM Artista")
    res = cursor.fetchall()
    artistasBase = [artista[0] for artista in res]
    artistas = []

    seguir = False
    multiples = ""
    while not seguir:
        print("Tiene varios artistas o solo uno?")
        print("1: Solo uno")
        print("2: Varios")
        multiples = input("Opción: ")
        if multiples == "1" or multiples == "2":
            seguir = True
        else:
            print("Opción inválida")

    while multiples == "2" or multiples == "1":
        if multiples == "1":
            multiples = 0
        print("Está el artista en la base de datos?")
        seguir = False
        while not seguir:
            print("1: Sí")
            print("2: No")
            print("3: Mostrar los artistas guardados")
            res = input("Opción: ")
            if res == "3":
                print()
                print('\n'.join(artistasBase))
                print()
            elif res == "1" or res == "2":
                seguir = True
            else:
                print("Opción inválida")

        if res == "1":
            seguir = False
            while not seguir:
                print("Escoge el artista de la lista:")
                i = 0
                print()
                for artista in artistasBase:
                    print(f"{i}: {artista}")
                    i += 1
                print()
                i -= 1
                num = input("Número del artista: ")
                if not num.isnumeric():
                    print("Eso no es un número")
                    continue

                num = int(num)
                if num < 0 or num > i:
                    print("Ese número no es válido")
                    continue

                cursor.execute("SELECT * FROM Artista WHERE Nombre = %s", [artistasBase[num]])
                artistas.append(cursor.fetchone()[1])

                seguir = True
        else:
            seguir = False
            nombre = ""
            while not seguir:
                nombre = input("Introduce el nombre del artista: ")
                print(f"Has introducido: {nombre}")

                if nombre in artistas:
                    print("Ese artista ya está en la base de datos")
                    continue

                print("¿Es eso correcto?")
                print("1: Sí")
                print("2: No")
                res = input("Opción: ")
                if res == "1":
                    seguir = True
                elif res != "2":
                    print("Opción inválida")

            cursor.execute("INSERT INTO Artista(Nombre) VALUES (%s)", [nombre])
            artistas.append(nombre)
                
        print(f"Artista(s) seleccionados: {artistas}")
        if multiples == "2":
            seguir = False
            while not seguir:
                print("Tiene algún otro artista?")
                print("1: Sí")
                print("2: No")
                res = input("Opción: ")
                if not res.isnumeric():
                    print("Es no es un número")
                elif res == "2":
                    seguir = True
                    multiples = "0"
                elif res == "1":
                    seguir = True
                else:
                    print("Opción inválida")

    return artistas

def conseguirGenero() -> str | None:
    seguir = False
    while not seguir:
        print("Quieres asignarle algún género a la canción?")
        print("1: Sí")
        print("2: No")
        res = input("Opción: ")
        if res == "1":
            seguir = True
        elif res == "2":
            return None
        else:
            print("Opción inválida")

    cursor.execute("SELECT Nombre FROM Genero")
    res = cursor.fetchall()
    generos = [genero[0] for genero in res]
    genero = ""
    
    seguir = False
    while not seguir:
        print("Está el género de la canción el la base de datos?")
        print("1: Sí")
        print("2: No")
        print("3: Mostrar géneros guardados")
        res = input("Opción: ")
        if res == "3":
            print()
            print('\n'.join(generos))
            print()
        elif res == "1" or res == "2":
            seguir = True
        else:
            print("Opción inválida")

    if res == "1":
        seguir = False
        while not seguir:
            print("Escoge el género de la lista:")
            i = 0
            print()
            for genero_ in generos:
                print(f"{i}: {genero_}")
                i += 1
            print()
            i -= 1
            num = input("Número del género: ")
            if not num.isnumeric():
                print("Eso no es un número")
                continue

            num = int(num)
            if num < 0 or num > i:
                print("Ese número no es válido")
                continue

            cursor.execute("SELECT * FROM Genero WHERE Nombre = %s", [generos[num]])
            genero = cursor.fetchone()[1]

            seguir = True
    else:
        seguir = False
        while not seguir:
            nombre = input("Introduce el nombre del género: ")
            print(f"Género introducido: {nombre}")
            
            if nombre in generos:
                print("Ese género ya existe")
                continue

            print("¿Es esto correcto?")
            print("1: Sí")
            print("2: No")
            res = input("Opción: ")
            if res == "1":
                seguir = True
            elif res != "2":
                print("Opción inválida") 


        cursor.execute("INSERT INTO Genero(Nombre) VALUES (%s)", [nombre])
        genero = nombre

    print(f"Género de la canción: {genero}")
    return genero

def conseguirAlbum() -> str | None:
    seguir = False
    albumes = []
    album = ""
    res: str = ""
    while not seguir:
        print("Está la canción en algún álbum?")
        print("1: Sí")
        print("2: No")
        res = input("Opción: ")
        if res == "2":
            return None
        elif res == "1":
            seguir = True
        else:
            print("Opción inválida")

    cursor.execute("SELECT Nombre FROM Album")
    albumes = [entrada[0] for entrada in cursor.fetchall()]
    
    seguir = False
    while not seguir:
        print("Está el álbum de la canción en la base de datos?")
        print("1: Sí")
        print("2: No")
        print("3: Ver álbumes guardados")
        res = input("Opción: ")
        if res == "1" or res == "2":
            seguir = True
        elif res == "3":
            print()
            print('\n'.join(albumes))
            print()
        else:
            print("Opción inválida")

    if res == "1":
        seguir = False
        while not seguir:
            print("Selecciona el álbum de la lista:")
            i = 0
            print()
            numero = -1
            for nombre in albumes:
                print(f"{i}: {nombre}")
                i += 1
            print()
            i -= 1

            res = input("Opción: ")
            if res.isnumeric():
                numero = int(res)
            else:
                print("Opción inválida")
                continue
            if numero < 0 or numero > i:
                print("Opción inválida")
            else:
                seguir = True
        album = albumes[i]
    else:
        seguir = False
        while not seguir:
            album = input("Introduce el nombre del álbum: ")
            if album in albumes:
                print("Ese álbum ya existe")
                continue
            print(f"Has introducido: {album}")
            print("Es eso correcto?")
            print("1: Sí")
            print("2: No")
            res = input("Opcíon: ")
            if res == "1":
                seguir = True
            elif res != "2":
                print("Opción inválida")
        
        seguir = False
        artistas = []
        while not seguir:
            print("Introduce los datos de los artistas del álbum")
            artistas = conseguirArtistas()
            seguir2 = False
            while not seguir2:
                print("Has introducido los siguientes artistas:")
                print()
                for artista in artistas:
                    print(f"- {artista}")
                print()
                print("Es esto correcto?")
                print("1: Sí")
                print("2: No")
                res = input("Opción: ")
                if res == "1":
                    seguir = True
                    seguir2 = True
                elif res == "2":
                    seguir2 = True
                else:
                    print("Opción inválida")

        año = conseguirAño()

        cursor.execute("INSERT INTO Album(Nombre, FechaPublicacion) VALUES (%s, %s)", [album, f"{año}-01-01"])
        cursor.execute("SELECT ID FROM Album WHERE Nombre = (%s)", [album])
        idAlbum = cursor.fetchone()[0]
        for artista in artistas:
            cursor.execute("SELECT ID FROM Artista WHERE NOMBRE = (%s)", [artista])
            idArtista = cursor.fetchone()[0]
            cursor.execute("INSERT INTO Artista_Album(ArtistaID, AlbumID) VALUES (%s, %s)", [idArtista, idAlbum])
            

    return album

def conseguirNombre(porPath: str) -> str:
    nombre = porPath
    seguir = False
    while not seguir:
        print("Es este el nombre de la canción?")
        print(f"Nombre: {nombre}")
        print("1: Sí")
        print("2: No")
        res = input("Opción: ")
        if res == "1":
            seguir = True
        elif res == "2":
            seguir = True
            nombre = ""
        else:
            print("Opción inválida")

    seguir = False
    while not seguir and nombre == "":
        nombre = input("Introduce el nombre de la canción: ")
        print(f"Has introducido: {nombre}")
        print("Es eso correcto?")
        print("1: Sí")
        print("2: No")
        res = input("Opción: ")
        if res == "1":
            seguir = True
        else:
            if res != "2":
                print("Opción inválida")
            nombre = ""

    return nombre

def conseguirAño() -> int:
    año: str = ""
    ret: int = 0
    seguir = False
    while not seguir:
        print("Introduce el año de publicación")
        print("Si no es conocido introduce 0")
        año = input("Año: ")
        if not año.isnumeric():
            print("Eso no es un número")
            continue
        
        ret = int(año)
        if ret < 1950 and año != 0:
            print("No creo que sea tan vieja...")
            continue

        print(f"Has introducido {ret}")
        print("Eso eso correcto?")
        print("1: Sí")
        print("2: No")
        res = input("Opción: ")
        if res == "1":
            seguir = True
        elif res != "2":
            print("Opción inválida")

    return int(año)

def conseguirDuracion(metadatos: dict[str, str | None] | None = None) -> int:
    duracion: int = 0
    if metadatos != None and metadatos["duration"] != None:
        duracion = int(metadatos["duration"])
        seguir = False
        while not seguir:
            print(f"Es la duración de la canción de {duracion} segundos?")
            print("1: Sï")
            print("2: No")
            res = input("Opcíon: ")
            if res == "1":
                seguir = True
            elif res == "2":
                seguir = True
                duracion = 0
            else:
                print("Opción inválida")

    if duracion == 0:
        seguir = False
        while not seguir:
            res = input("Introduce la duración de la canción: ")
            if not res.isnumeric():
                print("Eso no es un número")
                continue
            duracion = int(res)
            if duracion <= 0:
                print("Duración inválida")

            print(f"Has introducido que la duración es de {duracion} segundos")
            print("Es eso correcto?")
            print("1: Sí")
            print("2: No")
            res = input("Opción: ")
            if res == "1":
                seguir = True
            elif res != "2":
                print("Opción inválida")

    return duracion

def descargada(path: str, metadatos: dict[str, str | None] | None = None):
    album = conseguirAlbum()
    nombre = conseguirNombre('.'.join(path.split('/')[-1].split('.')[:-1]))
    artistas = conseguirArtistas()
    genero = conseguirGenero()
    año = conseguirAño()
    duracion = conseguirDuracion(metadatos)
    
    correcto = False
    while not correcto:
        print("Estos son los datos introducidos:")
        print()
        print(f"\tNombre: {nombre}")
        for artista in artistas:
           print(f"\tArtista: {artista}")
        print(f"\tGenero: {genero}")
        print(f"\tÁlbum: {album}")
        print(f"\tAño: {año}")
        print(f"\tDuración: {duracion}")
        print()
        seguir = False
        cambiar = False
        while not seguir:
            print("Es todo esto correcto o hay algo mal?")
            print("1: Todo correcto")
            print("2: Hay algo mal")
            res = input("Opción: ")
            if res == "1":
                seguir = True
                correcto = True
            elif res == "2":
                seguir = True
                cambiar = True
            else:
                print("Opción inválida")
        
        while cambiar:
            print("1: Nombre")
            print("2: Artistas")
            print("3: Género")
            print("4: Álbum")
            print("5: Año")
            print("6: Duración")
            print("0: Nada más")
            res = input("Opción: ")
            if res == "1":
                nombre = conseguirNombre()
            elif res == "2":
                artistas = conseguirArtistas()
            elif res == "3":
                genero = conseguirGenero()
            elif res == "4":
                album = conseguirAlbum()
            elif res == "5":
                año = conseguirAño()
            elif res == "6":
                duracion = conseguirDuracion(metadatos)
            elif res == "0":
                cambiar = False
            else:
                print("Opción inválida")
    
    albumId = None
    if album != None:
        cursor.execute("SELECT ID FROM Album WHERE Nombre = (%s)", [album])
        result = cursor.fetchone()
        albumId = result[0] if result else None

    generoId = None
    if genero != None:
        cursor.execute("SELECT ID FROM Genero WHERE Nombre = (%s)", [genero])
        result = cursor.fetchone()
        generoId = result[0] if result else None

    cursor.execute("INSERT INTO Cancion(AlbumID, GeneroID, Nombre, FechaPublicacion, Path, Duracion) VALUES (%s, %s, %s, %s, %s, %s)", [
        albumId,
        generoId,
        nombre,
        f"{año}-01-01",
        path,
        duracion
    ])

    cursor.execute("SELECT ID FROM Cancion WHERE Nombre = (%s)", [nombre])
    idCancion = cursor.fetchone()[0]
    for artista in artistas:
        cursor.execute("SELECT ID FROM Artista WHERE Nombre = (%s)", [artista])
        idArtista = cursor.fetchone()[0]
        cursor.execute("INSERT INTO Cancion_Artista(CancionID, ArtistaID) VALUES (%s, %s)", [idCancion, idArtista])

    print("¡Cancion añadida satisfactoriamente!")
    print()

    
def get_music_metadata(url) -> dict[str, str | None] | None:
    """
    Extract music-specific metadata from YouTube video
    
    Args:
        url (str): YouTube video URL
    
    Returns:
        dict: Music metadata with the following keys:
            - artist: Artist name (priority: official -> parsed from title -> uploader)
            - track_name: Track name (priority: official -> parsed from title)
            - album_name: Album name (if available)
            - cover_url: URL to video thumbnail (album/track cover)
            - release_year: Release year (priority: official -> video upload year)
            - duration: Duration in seconds
    """
    
    try:
        with yt_dlp.YoutubeDL({'quiet': True}) as ydl:
            info: None | dict[str, str] = ydl.extract_info(url, download=False)
            if not info:
                return None
            
            # Parse artist and track from video title
            parsed_info = _parse_music_from_title(info.get('title', ''))
            
            # Extract upload year from upload_date (format: YYYYMMDD)
            upload_year = None
            if info.get('upload_date'):
                try:
                    upload_year = int(str(info['upload_date'])[:4])
                except (ValueError, TypeError):
                    upload_year = None
            
            # Build metadata with fallback priority
            metadata: dict[str, str | None] = {
                'artist': (
                    info.get('artist') or 
                    parsed_info.get('artist') or 
                    info.get('uploader') or
                    info.get('channel') or
                    ''
                ),
                
                'track_name': (
                    info.get('track') or 
                    parsed_info.get('track') or 
                    info.get('title') or
                    ''
                ),
                
                'album_name': info.get('album'),
                
                'cover_url': info.get('thumbnail'),
                
                'release_year': (
                    info.get('release_year') or 
                    str(upload_year)
                ),
                
                'duration': info.get('duration')
            }
            
            return metadata
            
    except Exception as e:
        print(f"Error extracting metadata: {str(e)}")
        return None


def _parse_music_from_title(title):
    """
    Extract artist and track name from YouTube video title
    Handles common patterns like "Artist - Song", "Song by Artist", etc.
    
    Args:
        title (str): YouTube video title
    
    Returns:
        dict: Contains 'artist' and 'track' keys (can be None)
    """
    
    if not title:
        return {'artist': None, 'track': None}
    
    # Common YouTube music title patterns
    patterns = [
        r'^(.+?)\s*-\s*(.+)$',      # "Artist - Song"
        r'^(.+?)\s*:\s*(.+)$',      # "Artist: Song"  
        r'^(.+?)\s+by\s+(.+)$',     # "Song by Artist"
        r'^(.+?)\s*\|\s*(.+)$',     # "Artist | Song"
        r'^(.+?)\s*–\s*(.+)$',      # "Artist – Song" (em dash)
        r'^(.+?)\s*—\s*(.+)$',      # "Artist — Song" (long dash)
    ]
    
    for pattern in patterns:
        match = re.match(pattern, title.strip(), re.IGNORECASE)
        if match:
            part1, part2 = match.groups()
            part1, part2 = part1.strip(), part2.strip()
            
            # For "Song by Artist" pattern, swap the order
            if ' by ' in title.lower():
                return {
                    'artist': part2,
                    'track': part1
                }
            else:
                return {
                    'artist': part1,
                    'track': part2
                }
    
    # If no pattern matches, return None for artist and title as track
    return {
        'artist': None,
        'track': title.strip()
    }

def descargar(path: str, url: str) -> Optional[str]:
#    if not connection or not cursor:
#        return None
    
    output_dir = os.path.dirname(path)
    os.makedirs(output_dir, exist_ok=True)
    
    output_without_ext = os.path.splitext(path)[0]
    
    metadata = get_music_metadata(url)

    if metadata:
        print(metadata)
    else:
        print("No hay metadatos?")

    downloaded_file = None
    
    def hook(d):
        nonlocal downloaded_file
        if d['status'] == 'finished':
            downloaded_file = d['filename']
    
    ydl_opts = {
        'format': 'bestaudio/best',
        'outtmpl': output_without_ext + '.%(ext)s',  
        'postprocessors': [{
            'key': 'FFmpegExtractAudio',
            'preferredcodec': 'mp3',  
            'preferredquality': '0',  
        }],
        'progress_hooks': [hook]
    }
    
    try:
        with yt_dlp.YoutubeDL(ydl_opts) as ydl:
            print("Descargando...")
            ydl.download([url])
            print(f"Descargado en: {downloaded_file}")
            return downloaded_file
            
    except Exception as e:
        print(f"Error descargando: {str(e)}")
        return None


def interactivo():
#   if not connection or not cursor:
#        return
    
    entrada: str = ""
    path: str = ""

    while entrada != "3":
        entrada = ""
        while entrada != "1" and entrada != "2":
            print("1. Añadir canción existente en el sistema")
            print("2. Descargar canción al sistema y añadir")
            print("3. Terminar")
            entrada = input("Opción: ")

            if entrada == "3":
                return
        
        path = input("Introduce el path a la canción: ")
        
        if entrada == "1":
            descargada(path)
        else:
            path_real = descargar(path, input("Introduce la URL de la canción a descargar: "))
            if path_real != None:
                descargada(path)

def help():
    print("Esta herramienta sirve para añadir canciones ya descargadas en el sistema a la base de datos de GPlayer")
    print("y para descargar canciones desde internet y añadirla a la base de datos de GPlayer")
    print("Plataformas de descarga soportadas:")
    print("- YouTube")
    print("Uso:")
    print("\tGPlayer.py <path a la cancion>")
    print("\tGPlayer.py <path de destino de la cancion a descargar> <URL de la cancion a descargar>")
    print("Se puede usar de forma interactiva si no se pasan argumentos")

if __name__ == '__main__':
    if len(sys.argv) == 2:
        if not sys.argv[1].startswith("/"):
            help()
        else:
            connect()
            descargada(sys.argv[1])
    elif len(sys.argv) == 3:
        if not sys.argv[1].startswith("/") or not sys.argv[2].startswith("https://"):
            help()
        else:
            connect()
            descargar(sys.argv[1], sys.argv[2])
    elif len(sys.argv) != 1:
        help()
        sys.exit()

    connect()
    interactivo()
