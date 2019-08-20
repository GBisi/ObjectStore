#ifndef OBJSTORE_H
#define OBJSTORE_H

#include <stdlib.h>

/*
DESCRIPTION: Invia richiesta di connessione al server attraverso il file SOCKNAME. La connessione è globale.
PARAMETERS: name -> nome utente
RETURN VALUE: 1 se connessione effettuata, 0 altrimenti
*/
int os_connect(char *name);

/*
DESCRIPTION: Richiede al server di memorizzare un blocco di dati
PARAMETERS: name -> nome blocco, block -> dati da memorizzare, len -> dimensione dati (in byte)
RETURN VALUE: 1 se richiesta soddisfatta, 0 altrimenti
*/
int os_store(char *name, void *block, size_t len);

/*
DESCRIPTION: Richiede al server di recuperare un blocco di dati
PARAMETERS: name -> nome blocco
RETURN VALUE: 1 se richiesta soddisfatta, 0 altrimenti
*/
void* os_retrieve(char *name);

/*
DESCRIPTION: Richiede al server di eliminare un blocco di dati
PARAMETERS: name -> nome blocco
RETURN VALUE: 1 se richiesta soddisfatta, 0 altrimenti
*/
int os_delete(char *name);

/*
DESCRIPTION: Invia richiesta di disconnessione al server
RETURN VALUE: 1 se richiesta soddisfatta, 0 altrimenti
*/
int os_disconnect();

#endif

