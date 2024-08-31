#include <iostream>
#include <thread>
#include <WS2tcpip.h>
#include<vector>
#include <sstream>


using namespace std;


#pragma comment(lib, "ws2_32.lib")



class rsa {
    long long int e; // public key
    long long int d; // private key
    long long int modulus;

public:
    rsa(long long int enc, long long int dec, long long int modul) {
        e = enc;
        d = dec;
        modulus = modul;
    }

    string encrypt_it(const string& u_input) {
        vector<long long int> cnum_arr(u_input.size());
        ostringstream oss;

        for (size_t k = 0; k < u_input.size(); ++k) {
            long long int temp = u_input[k];
            cnum_arr[k] = mod_exp(temp, e, modulus);
            oss << cnum_arr[k];
            if (k < u_input.size() - 1)
                oss << " ";  // Using space as delimiter
        }

        return oss.str();
    }

    string decrypt_it(const string& encrypted_input) {
        istringstream iss(encrypted_input);
        vector<long long int> encrypted_numbers;
        long long int number;

        while (iss >> number) {
            encrypted_numbers.push_back(number);
        }

        string u_input(encrypted_numbers.size(), ' ');

        for (size_t k = 0; k < encrypted_numbers.size(); ++k) {
            long long int temp = encrypted_numbers[k];
            long long int decrypted_val = mod_exp(temp, d, modulus);
            u_input[k] = static_cast<char>(decrypted_val);
        }

        return u_input;
    }

    long long int mod_exp(long long int base, long long int exponent, long long int modulus) {
        long long int result = 1;
        while (exponent > 0) {
            if (exponent % 2 == 1) {
                result = (result * base) % modulus;
            }
            base = (base * base) % modulus;
            exponent = exponent / 2;
        }
        return result;
    }
};

string getSubstringAfterColon(const string& input) {
    size_t pos = input.find(':');  // Find the position of the colon
    if (pos != string::npos) {
        return input.substr(pos + 1);  // Extract substring after the colon
    }
    else {
        return "";  // Return an empty string if the colon is not found
    }
}

void receiveMessages(SOCKET sock) {
    char buf[4096];
    while (true) {
        ZeroMemory(buf, 4096);
        int bytesReceived = recv(sock, buf, 4096, 0);
        if (bytesReceived > 0) {
            rsa rsa1(5, 167741, 840551);
            string decrypted = getSubstringAfterColon(string(buf, 0, bytesReceived));

            //  cout << "lullo" << decrypted << "lullo" << endl;


            cout << "SERVER> " << rsa1.decrypt_it(decrypted) << endl;
            // cout << "SERVER> " <<  string(buf, 0, bytesReceived) << endl;

        }
        else {
            // If recv returns 0, the connection was closed
            cout << "Server connection closed." << endl;
            break;
        }
    }
}



int main() {
    // Initialize Winsock
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    int wsOk = WSAStartup(ver, &wsData);
    if (wsOk != 0) {
        cerr << "Can't initialize Winsock! Quitting" << endl;
        return -1;
    }

    // Create a socket
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Can't create socket! Quitting" << endl;
        WSACleanup();
        return -1;
    }

    // Fill in a hint structure
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    inet_pton(AF_INET, "172.20.10.5", &hint.sin_addr); // localhost

    // Connect to the server
    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR) {
        cerr << "Can't connect to server! Quitting" << endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    // Start the receive thread
    thread recvThread(receiveMessages, sock);

    // Do-while loop to send data
    string userInput;
    do {
        // Prompt the user for some text
        cout << "> ";
        getline(cin, userInput);

        if (userInput.size() > 0) { // Make sure the user has typed something
            // Send the text

            //encrypting text !!!!!!!!!!!!!!!!!!!!!!
            rsa rsa1(5, 167741, 840551);
            string encrypted = rsa1.encrypt_it(userInput);
            userInput = encrypted;
            //
            int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
            if (sendResult == SOCKET_ERROR) {
                cerr << "Failed to send message. Quitting" << endl;
                break;
            }
        }
    } while (userInput.size() > 0);

    // Clean up
    closesocket(sock);
    WSACleanup();
    recvThread.join();

    return 0;
}
