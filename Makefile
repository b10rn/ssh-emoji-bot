PROGRAM := "ssh-emoji-bot"

$(PROGRAM): main.c
	gcc -o $(PROGRAM) main.c -lssh

clean:
	rm -f $(PROGRAM)
