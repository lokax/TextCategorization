Problem_A1:problem_a1.cpp problem_a2.cpp
	g++ problem_a1.cpp -std=c++17 -O3 -o Problem_A_1
	g++ problem_a2.cpp -std=c++17 -O3 -o Problem_A_2

clean:
	rm -rf Problem_A_1 Problem_A_2