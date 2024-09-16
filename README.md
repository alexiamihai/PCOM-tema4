# ReadMe - Aplicatie Client de Gestionare a Cartilor

## Introducere

Acest proiect este o aplicatie client scrisa in C care comunica cu un server pentru a gestiona o colectie de
carti. Aplicatia ofera functionalitati precum inregistrarea utilizatorilor, autentificare, accesarea unei
biblioteci, vizualizarea cartilor, adaugarea de noi carti si stergerea cartilor. Utilizeaza tehnici de
programare de retea si parsare JSON pentru a interactiona cu serverul prin cereri HTTP.


## Fisiere

- `client.c`: Fisierul sursa principal care contine implementarea aplicatiei.
- `helpers.h`: Fisierul header care contine declaratiile functiilor helper.
- `requests.h`: Fisierul header pentru gestionarea cererilor HTTP, POT, GET, DELETE.
- `parson.h` si `parson.c`: Fisierele pentru biblioteca Parson, folosite pentru parsarea JSON.


## Functionalitati

In fisierul client.c, inainte de fiecare comanda primita de la tastatura conform enuntului (cum ar fi login,
logout, add_book, delete_book, enter_library, register), se deschide o conexiune noua cu serverul de fiecare
data cand o comanda este introdusa la stdin. Apoi, in momentul in care se primeste comanda exit, conexiunea cu
serverul este inchisa.

### 1. Inregistrare Utilizator Nou

Comanda: `register`

- Solicita introducerea unui nume de utilizator si a unei parole.
- Valideaza inputul (nu sunt permise spatii).
- Trimite o cerere de inregistrare in format JSON catre server.

### 2. Autentificare

Comanda: `login`

- Solicita introducerea unui nume de utilizator si a unei parole.
- Valideaza inputul (nu sunt permise spatii).
- Trimite o cerere de autentificare in format JSON catre server.
- Proceseaza raspunsul serverului pentru a prelua si stoca cookie-urile de sesiune.

### 3. Deconectare

Comanda: `logout`

- Deconecteaza utilizatorul curent prin invalidarea cookie-urilor de sesiune.

### 4. Accesare Biblioteca

Comanda: `enter_library`

- Solicita acces la biblioteca.
- Proceseaza raspunsul serverului pentru a prelua si stoca un token de acces.

### 5. Obtinere Lista Cartilor

Comanda: `get_books`

- Recupereaza si afiseaza o lista cu toate cartile disponibile in biblioteca.

### 6. Obtinere Detalii Carte

Comanda: `get_book`

- Solicita introducerea unui ID de carte.
- Recupereaza si afiseaza detaliile cartii specificate.

### 7. Adaugare Carte Noua

Comanda: `add_book`

- Solicita introducerea detaliilor cartii (titlu, autor, gen, editor, numar de pagini).
- Valideaza inputul.
- Trimite o cerere in format JSON pentru a adauga cartea in biblioteca.

### 8. Stergere Carte

Comanda: `delete_book`

- Solicita introducerea unui ID de carte.
- Valideaza ID-ul primit.
- Trimite o cerere pentru a sterge cartea specificata din biblioteca.

### 9. Iesire

Comanda: `exit`

- Inchide aplicatia.

## Parsarea JSON cu Biblioteca Parson

### De ce am ales Parson?

Am optat pentru biblioteca Parson datorita simplitatii si usurintei sale de integrare in proiectele scrise in limbajul C. Aceasta ofera un API intuitiv pentru crearea, modificarea si parsarea obiectelor JSON, ceea ce este
esential pentru interactiunea aplicatiei noastre cu serverul.

### Crearea Obiectelor JSON

Functia `create_json_object` exemplifica modul in care putem crea dinamic obiecte JSON in functie de campurile
de intrare. Aceasta functie este folosita pentru a construi payload-uri JSON pentru cererile de inregistrare,
autentificare si gestionare a cartilor.

### Parsarea Raspunsurilor JSON

Parson este utilizat si pentru a parsa raspunsurile primite de la server. De exemplu, la autentificare,
aplicatia proceseaza raspunsul pentru a extrage cookie-urile de sesiune. Similar, la accesarea bibliotecii,
parseaza raspunsul pentru a prelua token-ul de acces.

## Gestionarea Erorilor

Aplicatia include o gestionare de baza a erorilor pentru a asigura un comportament robust. Validarile inputului
previn trimiterea datelor incorecte sau malformate catre server. In plus, raspunsurile primite de la server sunt
verificate pentru a determina succesul sau esecul unei cereri, iar utilizatorului ii sunt afisate mesajele
corespunzatoare.

### Testare

Am testat pentru username-ul "barbie" cu parola "slay" și testele imi trec, cel putin din cate am testat eu, atat
cu checker-ul, cat si local.

## Timp de implementare

Am rezolvat tema in 2 zile.

# Mentiuni

Pentru implementare, am pornit de la rezolvarea laboratorului 10 si am adaptat codul la cerintele temei.
In special, am utilizat fisierele pentru request-uri si cele cu functii helper.


## Concluzie

Acest proiect demonstreaza o aplicatie practica de programare de retea si manipulare a datelor JSON in C.
Prin utilizarea bibliotecii Parson, obtinem un cod eficient si usor de intretinut pentru gestionarea datelor
JSON, facand aplicatia fiabila si usor de extins cu noi functionalitati.