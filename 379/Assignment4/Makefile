

default: compile

compile: 
	# g++ -o a2sdn  a2sdn.cc
	g++  a4tasks.cc -lpthread -o a4tasks

test: test_RemoveDigits
	./test_RemoveDigits

test_RemoveDigits: test_RemoveDigits.cc a4tasks.cc
	g++ test_RemoveDigits.cc -lpthread -o test_RemoveDigits

eclass:
	gcc -o raceA raceA.c -lpthread
	gcc -o raceB raceB.c -lpthread
	gcc -o raceC raceC.c -lpthread
	gcc -o raceD raceD.c -lpthread
	gcc -o raceE raceE.c -lpthread
clean_eclass:
	rm raceA
	rm raceB
	rm raceC
	rm raceD
	rm raceE

.PHONY: test_linked_list
test_linked_list: test_linked_list.c linked_list.c
	gcc -o test_linked_list test_linked_list.c
	./test_linked_list

tar:
	tar -cvf submit.tar *

clean:
	find . -type f ! -name "*.html" ! -name "*.dat" ! -name "*.c" ! -name "*.cc" ! -name "*.o" ! -name "Makefile" -exec rm {} \; 
