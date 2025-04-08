# Proyecto: Servidor Web en C

Este proyecto implementa un servidor HTTP en lenguaje C, capaz de manejar solicitudes **GET**, **HEAD** y **POST**, con soporte completo para archivos estáticos, reproducción de video por partes (streaming) y almacenamiento de datos recibidos por formularios.

---

## 🔧 Requisitos

- Ubuntu o WSL (Windows Subsystem for Linux)
- GCC para compilar el servidor
- Navegador web y `curl` para pruebas

---

## ✅ Compilación y ejecución

1. Abre WSL y navega a tu carpeta del proyecto:

```bash
cd /mnt/c/Users/andru/Videos/webserver
```

2. Compila el servidor:

```bash
make
```

3. Ejecuta el servidor:

```bash
./server 8080 log.txt www
```

Esto:
- Usa el puerto 8080
- Registra las solicitudes en `log.txt`
- Sirve archivos desde la carpeta `www/`

---

## 📁 Estructura del proyecto

```
webserver/
├── server.c         # Servidor HTTP principal
├── logger.c/.h      # Registro de solicitudes
├── Makefile         # Compilación automatizada
├── log.txt          # Registro de todas las peticiones
├── postdata.txt     # Datos recibidos por formularios POST
└── www/             # Carpeta con los recursos estáticos
    ├── caso1.html
    ├── caso2.html
    ├── caso3.html
    ├── caso4.html
    ├── Olvidate.mp4
    ├── Olvidate2.mp4
    ├── style.css
    ├── script.js
    ├── images.jpeg
    └── descarga.jpeg
```

---

## 🔄 Casos implementados

### Caso 1: Imagen y enlaces
- HTML con una imagen (`descarga.jpeg`) y enlaces a otros casos.

### Caso 2: Reproducción de videos en loop
- Dos videos (`Olvidate.mp4`, `Olvidate2.mp4`) con autoplay y bucle.
- Streaming compatible con `Range` y `206 Partial Content`

### Caso 3: Recursos estáticos + formulario
- Usa CSS, JS, imagen decorativa.
- Formulario HTML que envía datos por `POST`.
- Los datos se guardan en `postdata.txt`.

### Caso 4: Archivo grande
- `large.txt` de ~1MB para probar descargas grandes.

---

## 🔀 Pruebas con `curl`

### 🔷 GET
```bash
curl -i http://localhost:8080/caso1.html
```

### 🔷 HEAD
```bash
curl -I http://localhost:8080/caso3.html
```

### 🔷 POST
```bash
curl -X POST http://localhost:8080/caso3.html -d "nombre=andru"
```

### 🔷 STREAMING (Rango)
```bash
curl -H "Range: bytes=0-999" http://localhost:8080/Olvidate.mp4 -i
```

---

## 🔮 Registro de actividad

Ejemplo en `log.txt`:
```
127.0.0.1 GET /caso1.html 200
127.0.0.1 POST /caso3.html 200
127.0.0.1 GET /Olvidate.mp4 206
127.0.0.1 HEAD /caso2.html 200
```

Contenido en `postdata.txt`:
```
[POST] /caso3.html - nombre=andru
```

---

## 🚀 Funcionalidades extra
- Streaming de video por rango (`Accept-Ranges` y `206 Partial Content`)
- Autoplay y loop en videos
- Respuesta HTML personalizada al enviar formularios
- Registro y almacenamiento de datos `POST`

---

## 📊 Códigos HTTP implementados

| Código | Descripción                  |
|--------|------------------------------|
| 200    | OK                          |
| 206    | Partial Content (video)     |
| 400    | Bad Request (método inválido) |
| 404    | Not Found (archivo faltante)|

---

Proyecto de Web Server para Telemática 2025

