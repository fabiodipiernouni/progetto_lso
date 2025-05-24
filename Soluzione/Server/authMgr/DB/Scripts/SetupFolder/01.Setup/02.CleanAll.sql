
begin
    execute immediate 'DROP sequence utente_seq';
    dbms_output.put_line('Utente dropped');
exception when others then
    dbms_output.put_line('WARNING: tabella Utente non esistente.');
end;

begin
    execute immediate 'DROP TABLE Utente CASCADE CONSTRAINTS PURGE';
    dbms_output.put_line('Utente dropped');
exception when others then
    dbms_output.put_line('WARNING: tabella Utente non esistente.');
end;


begin
    execute immediate 'DROP TABLE lingue CASCADE CONSTRAINTS PURGE';
    dbms_output.put_line('Utente dropped');
exception when others then
    dbms_output.put_line('WARNING: tabella Utente non esistente.');
end;

