# ft_irc — Plan de trabajo de Carlos Daureo

Mi parte del proyecto cubre el estado de cada cliente, la autenticación, el parser IRC, las respuestas del servidor y los comandos `PASS`, `NICK`, `USER`, `PING`, `QUIT` y `PRIVMSG` directo.

**Primer objetivo:** conseguir que `PING :prueba` devuelva `PONG` con el mismo token. Así compruebo que puedo recibir una línea, interpretarla y encolar una respuesta antes de implementar el registro.

## Dónde empezar en el repositorio

- [`Client.hpp`](https://github.com/LuaV16/irc/blob/658b72196eec3c998df8112500bf60db4195da9c/Client.hpp) y [`Client.cpp`](https://github.com/LuaV16/irc/blob/658b72196eec3c998df8112500bf60db4195da9c/Client.cpp) ya guardan los búferes de entrada y salida de cada cliente. `getCompleteLine()` extrae una línea completa y le quita el `\r\n`.
- [`Server::handleClientData()`] ya recorre las líneas completas. El comentario `CARLOS` marca el punto donde tengo que llamar al parser.
- [`Server::queueClientOutput()`] es la función que uso para encolar las respuestas. Cada mensaje IRC que le pase debe terminar en `\r\n`.
- [`Makefile`] compila con C++98. Si añado un archivo `.cpp`, tengo que incluirlo en `SRC`.

## 1. Parser y `PING`

- [ ] Crear mi rama desde `main`.
- [ ] Hacer un parser que reciba una línea completa y devuelva el comando y sus parámetros.
- [ ] Conservar como un solo parámetro el texto que empieza después del `:` que introduce el parámetro final, incluidos sus espacios.
- [ ] Conectar el parser en `handleClientData()` y responder a `PING` mediante `queueClientOutput()`.

**Ejemplo para probar el parser**

```text
Entrada:    PRIVMSG luis :hola, ¿qué tal?
Comando:    PRIVMSG
Parámetro 1: luis
Parámetro 2: hola, ¿qué tal?
```

**Lo doy por terminado cuando** `PING :prueba` devuelve `PONG`, una línea partida entre dos lecturas se procesa al completarse y dos comandos recibidos juntos se procesan por separado.

## 2. Estado del cliente y registro

- [ ] Añadir a `Client` la contraseña validada, el nickname, el username y el estado de registro. Cada cliente empieza sin registrar y con la contraseña pendiente de validar.
- [ ] Implementar `PASS` usando la contraseña que ya guarda `Server`. Devolver `464` si es incorrecta.
- [ ] Implementar `NICK` y comprobar que el nickname está disponible. Devolver `433` si ya lo usa otro cliente.
- [ ] Implementar `USER` para guardar el nombre de usuario.
- [ ] Centralizar la comprobación de registro. Cuando la contraseña sea válida y estén configurados el nickname y el username, marcar al cliente como registrado y enviar `001` una sola vez.
- [ ] Quitar el log de la línea recibida que hay junto al comentario `CARLOS`, porque imprimiría la contraseña enviada con `PASS`.

**Lo doy por terminado cuando** el servidor envía `001` al completar el registro, rechaza una contraseña incorrecta y detecta dos clientes que intentan usar el mismo nickname.

## 3. Respuestas numéricas y mensajes privados

- [ ] Centralizar el formato de las respuestas numéricas. Usar el nombre del servidor como prefijo, el nickname del destinatario o `*` si todavía carece de uno, y `\r\n` al final.
- [ ] Buscar al destinatario por nickname en `clientList`.
- [ ] En `PRIVMSG`, comprobar que el remitente está registrado (`451`), que existe el destinatario (`401`) y que hay texto (`412`).
- [ ] Construir el mensaje con el prefijo del remitente y encolarlo para el destinatario.

**Lo doy por terminado cuando** Ana puede enviar `PRIVMSG luis :hola, ¿qué tal?`, Luis recibe el texto completo y puedo comprobar cada error por separado.

## 4. `QUIT` y prueba con el cliente IRC

- [ ] Interpretar el motivo opcional de `QUIT` y solicitar la desconexión a través de la lógica de `Server`.
- [ ] Acordar con Persona 1 cuándo retirar al cliente. `clientList` es un `std::vector` y el bucle de lectura sigue usando un puntero al cliente mientras procesa sus líneas.
- [ ] Comprobar que el nickname queda disponible después de `QUIT` y que los demás clientes siguen conectados.
- [ ] Probar el registro, los mensajes privados y la salida con el cliente IRC de referencia del grupo. Revisar qué comandos envía al conectarse.

## Qué coordinar con el equipo

- **Persona 1 — Red.** Mi parte recibe líneas completas, encola respuestas y solicita la desconexión. Su parte gestiona `poll()`, `recv()`, `send()` y el cierre de la conexión.
- **Persona 3 — Canales.** Compartimos el parser y la comprobación de registro. Yo resuelvo `PRIVMSG` dirigido a un nickname. Si el destino es `#canal`, delego la entrega a la lógica de canales. También tenemos que acordar cómo notificar a los canales los cambios de nickname y las salidas.

**Primer commit sugerido:** `Conectar el parser IRC y responder a PING`

[Referencia de protocolo IRC — RFC 2812](https://www.rfc-editor.org/rfc/rfc2812)