# Nom du compilateur
CC = gcc
# Options de compilation
CFLAGS = -Wall
# Bibliothèques à lier
LIBS = -lpthread

# Nom de l'exécutable pour le client et le serveur
CLIENT = FTP_Client
SERVER = FTP_Server

# Fichiers sources
SRCS_CLIENT = FTP_Client.c csapp.c FTP_Log.c Signal_Handler_Client.c
SRCS_SERVER = FTP_Server.c FTP_Service.c Transfert_Fichier.c Signal_Handler_Server.c csapp.c

# Fichiers objets générés
OBJS_CLIENT = $(SRCS_CLIENT:.c=.o)
OBJS_SERVER = $(SRCS_SERVER:.c=.o)

# Règle principale
all: $(CLIENT) $(SERVER)

# Compilation du client
$(CLIENT): $(OBJS_CLIENT)
	$(CC) $(CFLAGS) -o $@ $(OBJS_CLIENT) $(LIBS)

# Compilation du serveur
$(SERVER): $(OBJS_SERVER)
	$(CC) $(CFLAGS) -o $@ $(OBJS_SERVER) $(LIBS)

# Compilation des fichiers sources en objets (pour le client et le serveur)
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Nettoyage des fichiers compilés
clean:
	rm -f $(OBJS_CLIENT) $(OBJS_SERVER) $(CLIENT) $(SERVER)
