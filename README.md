<h1 align="center">International Whispers</h1> <br>
<p align="center">
    <img alt="International Whispers logo" title="International Whispers logo" src="./Soluzione/Client/src/main/resources/org/internationalwhispers/client/presentation/images/icons/logo.png" width="450">
</p>

<p align="center">
    Project developed for the <i>Operating Systems Laboratory</i> course at the <i>University of Naples Federico II</i> in the academic year 2024-2025.
</p>
<p>
    <strong>Authors</strong>
    <ul align="left">
        <li><a href="https://github.com/giuseppedima">Giuseppe DI MARTINO - N86004948</a></li>
        <li><a href="https://github.com/fabiodipiernouni">Fabio DI PIERNO - N86004852</a></li>
    </ul>
    <strong>Teacher</strong>
    <ul align="left">
        <li><a href="https://www.docenti.unina.it/alessandra.rossi">Alessandra ROSSI</a></li>
    </ul>
</p>

# 🇬🇧

International Whispers is a simulation of the game "Chinese whispers" (also known as "Telephone"), a traditional game in which participants sequentially transmit a message, with the risk that it becomes altered along the way. The goal of this project is to implement a client-server system that allows users to play this game without worrying about the language spoken by other players; the system automatically translates the words into each player's language, thus removing any communication barriers.

## GitHub

The full source code for this project is available on GitHub. You are free to browse and review the code at the following link:

[https://github.com/fabiodipiernouni/progetto_lso](https://github.com/fabiodipiernouni/progetto_lso)

## Project Structure

- **app/**: Contains installation and launch scripts for the JavaFX application "International Whispers". All scripts are compatible with Ubuntu Linux.
  - **ubuntu/**: Contains:
    - `setup.sh`: Installs Java 21 (if not present), JavaFX 21 (if not present), and the required libraries for JavaFX. Run this script before launching the application.
    - `launch.sh`: Launches one or more instances of the client application after verifying that the server is running. Optionally accepts as a parameter the number of client instances to launch.
    - `launch_partita_10_e_1_coda.sh`: Launches 10 client instances (after verifying the server is up), immediately placing them in a game room, and then launches an additional client to join the queue for that room.
    - `IWClient-1.0.jar`: The compiled JavaFX client application.

- **Documentazione/**: Contains project documentation. Currently includes a Word document; at delivery, this will be replaced by a PDF.

- **src/**: Contains all project source code.
  - **Client/**: Java source code for the client application.
  - **Server/**: C source code for the server microservices, organized by service (e.g., `authMgr`, `gameRulesMgr`, `translatorSrv`).

## How to Run the Application (Ubuntu Linux)

1. Open a terminal and navigate to `app/ubuntu/`.
2. Run the installation script:
   ```bash
   ./setup.sh
   ```
   This will ensure Java 21, JavaFX 21, and all required libraries are installed.
3. To launch the client application:
   ```bash
   ./launch.sh [number_of_clients]
   ```
   Replace `[number_of_clients]` with the desired number of client instances (default is 1). The script checks if the server is running before launching clients.
4. To simulate a game scenario with multiple clients:
   ```bash
   ./launch_partita_10_e_1_coda.sh
   ```
   This will launch 10 clients into a game room and an additional client into the queue for that room.

---

# 🇮🇹

International whispers è una simulazione del gioco "Telefono senza fili" (in inglese Chinese whispers), un gioco tradizionale in cui i partecipanti si trasmettono un messaggio in modo sequenziale, con il rischio che esso venga alterato lungo il percorso. L’obiettivo del progetto è implementare un sistema client-server che permetta agli utenti di giocare a tale gioco senza preoccuparsi della lingua parlata dagli altri giocatori; infatti, il sistema si occupa di tradurre le parole nella lingua parlata da ogni giocatore abolendo così ogni difficoltà di comunicazione.

## GitHub

Il codice sorgente completo di questo progetto è disponibile su GitHub. Chiunque è libero di visionare e consultare il codice al seguente link:

[https://github.com/fabiodipiernouni/progetto_lso](https://github.com/fabiodipiernouni/progetto_lso)


## Struttura delle cartelle

- **app/**: Contiene gli script di installazione e avvio per l'applicativo JavaFX "International Whispers". Tutti gli script sono compatibili con Ubuntu Linux.
  - **ubuntu/**: Contiene:
    - `setup.sh`: Installa Java 21 (se non presente), JavaFX 21 (se non presente) e le librerie necessarie per JavaFX. Eseguire questo script prima di avviare l'applicazione.
    - `launch.sh`: Avvia una o più istanze del client dopo aver verificato che il server sia attivo. Accetta opzionalmente come parametro il numero di istanze client da avviare.
    - `launch_partita_10_e_1_coda.sh`: Avvia 10 client (dopo aver verificato che il server sia attivo) e li inserisce subito in una stanza di partita, quindi avvia un ulteriore client per inserirlo nella coda di quella stanza.
    - `IWClient-1.0.jar`: L'applicativo client JavaFX compilato.

- **Documentazione/**: Contiene la documentazione del progetto. Attualmente è presente un documento Word; alla consegna sarà sostituito da un PDF.

- **src/**: Contiene tutto il codice sorgente del progetto.
  - **Client/**: Codice sorgente Java per il client.
  - **Server/**: Codice sorgente C per i microservizi server, organizzati per servizio (es. `authMgr`, `gameRulesMgr`, `translatorSrv`).

## Istruzioni per l'esecuzione (Ubuntu Linux)

1. Aprire un terminale e posizionarsi in `app/ubuntu/`.
2. Eseguire lo script di installazione:
   ```bash
   ./setup.sh
   ```
   Questo script installerà Java 21, JavaFX 21 e tutte le librerie necessarie.
3. Per avviare il client:
   ```bash
   ./launch.sh [numero_client]
   ```
   Sostituire `[numero_client]` con il numero desiderato di istanze client (default 1). Lo script verifica che il server sia attivo prima di avviare i client.
4. Per simulare uno scenario di gioco con più client:
   ```bash
   ./launch_partita_10_e_1_coda.sh
   ```
   Questo comando avvia 10 client in una stanza di partita e un ulteriore client in coda per quella stanza.
