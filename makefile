all: project test

.outputFolder:
	mkdir -p out

project: .outputFolder
	clang -std=c++17 -lstdc++ -lm -Iinclude/ project.cpp -Wall -Wextra -Werror -O3 -ltbb -o out/project
	./out/project

test: .outputFolder
	clang -std=c++17 -Wall -Wextra -Werror -lstdc++ -lm -Iinclude/ tests.cpp -o out/tests
	./out/tests
