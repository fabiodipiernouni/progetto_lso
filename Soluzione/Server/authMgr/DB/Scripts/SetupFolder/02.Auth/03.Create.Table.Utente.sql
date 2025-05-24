CREATE TABLE utente (
    id_utente NUMBER(10) PRIMARY KEY,
    nome VARCHAR2(128) NOT NULL,
    cognome VARCHAR2(128) NOT NULL,
    email VARCHAR2(128) NOT NULL,
    password VARCHAR2(512) NOT NULL,
    codice_lingua VARCHAR2(10) NOT NULL,
    data_creazione timestamp DEFAULT CURRENT_TIMESTAMP NOT NULL,
    data_registrazione date DEFAULT SYSDATE NOT NULL
);

CREATE UNIQUE INDEX utente_idx_email ON utente(email);

create sequence utente_seq start with 1 increment by 1;

ALTER TABLE utente ADD CONSTRAINT fk_utente_lingue FOREIGN KEY (codice_lingua) REFERENCES lingue(codice);
