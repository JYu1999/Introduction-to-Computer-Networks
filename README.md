# Introduction-to-Computer-Networks(TCP Socket)
清華大學 NTHU(National Tsing Hua University)
11010EECS302001 計算機網路概論(Introduction to Computer Networks) 林華君教授
Final Project

建議使用Linux編譯。

## 說明
Implement a simple number guessing game. The server should allow clients  to guess a number between 0 to 999, and reply a guessing result. The client should be able to access the server, guess a number, and read the result replied by the server. Please use TCP socket, and the program should be  written in C or C++. You are also required to use Wireshark to capture the packets transmitted by the server and the client, observe the result, and answer the questions. 

### Function of Server Program
* Generate a random number for clients to guess.
* Allow clients to access the server.
* Show a prompt, and let the client guess a number. 
* Accept a client’s request and receive the guessing number from the client. 
* Respond the result to the clients.

### Function of Client Program
* Should be able to connect to the server. 
* Should be able to send a guessing number to the server. 
* Should be able to receive a response from the server.


## 使用步驟
1. g++ -g -o ser server.cpp -lpthread
2. g++ -g -o cli client.cpp -lpthread
3. ./ser 11800
4. ./cli 127.0.0.1 11800
(第4步的IP可用自己的代替)

其他說明文件可參考Document。
