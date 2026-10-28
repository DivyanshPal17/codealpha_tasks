// CodeAlpha C++ Internship - Task 2: Login and Registration System
// Credentials are stored in "users.db" as: username salt sha256(salt + password)
// Passwords are never stored in plain text.

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <random>
#include <cstdint>
#include <cctype>
#include <limits>

#ifdef _WIN32
  #include <conio.h>
#else
  #include <termios.h>
  #include <unistd.h>
#endif

using namespace std;

const string DB_FILE = "users.db";

// ===================== SHA-256 =====================
namespace sha {
const uint32_t K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};

inline uint32_t rotr(uint32_t x, int n) { return (x >> n) | (x << (32 - n)); }

string hash(const string &msg) {
    uint32_t h[8] = {0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,
                     0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
    vector<uint8_t> data(msg.begin(), msg.end());
    uint64_t bitLen = static_cast<uint64_t>(data.size()) * 8;
    data.push_back(0x80);
    while (data.size() % 64 != 56) data.push_back(0);
    for (int i = 7; i >= 0; --i) data.push_back(static_cast<uint8_t>(bitLen >> (i * 8)));

    for (size_t off = 0; off < data.size(); off += 64) {
        uint32_t w[64];
        for (int i = 0; i < 16; ++i)
            w[i] = (uint32_t(data[off + 4*i]) << 24) | (uint32_t(data[off + 4*i + 1]) << 16) |
                   (uint32_t(data[off + 4*i + 2]) << 8) | uint32_t(data[off + 4*i + 3]);
        for (int i = 16; i < 64; ++i) {
            uint32_t s0 = rotr(w[i-15], 7) ^ rotr(w[i-15], 18) ^ (w[i-15] >> 3);
            uint32_t s1 = rotr(w[i-2], 17) ^ rotr(w[i-2], 19) ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }
        uint32_t a=h[0], b=h[1], c=h[2], d=h[3], e=h[4], f=h[5], g=h[6], hh=h[7];
        for (int i = 0; i < 64; ++i) {
            uint32_t S1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
            uint32_t ch = (e & f) ^ (~e & g);
            uint32_t t1 = hh + S1 + ch + K[i] + w[i];
            uint32_t S0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t t2 = S0 + maj;
            hh = g; g = f; f = e; e = d + t1; d = c; c = b; b = a; a = t1 + t2;
        }
        h[0]+=a; h[1]+=b; h[2]+=c; h[3]+=d; h[4]+=e; h[5]+=f; h[6]+=g; h[7]+=hh;
    }
    ostringstream out;
    for (uint32_t v : h) out << hex << setw(8) << setfill('0') << v;
    return out.str();
}
} // namespace sha

// ===================== Helpers =====================
string generateSalt(size_t len = 16) {
    static const char hexChars[] = "0123456789abcdef";
    random_device rd;
    string salt;
    for (size_t i = 0; i < len; ++i) salt += hexChars[rd() % 16];
    return salt;
}

// Hash is stretched a number of rounds to slow down brute-force attempts
string hashPassword(const string &password, const string &salt) {
    string digest = sha::hash(salt + password);
    for (int i = 0; i < 5000; ++i) digest = sha::hash(digest + salt);
    return digest;
}

bool constantTimeEquals(const string &a, const string &b) {
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (size_t i = 0; i < a.size(); ++i) diff |= static_cast<unsigned char>(a[i] ^ b[i]);
    return diff == 0;
}

string toLower(string s) {
    for (char &c : s) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return s;
}

string readPassword(const string &prompt) {
    cout << prompt << flush;
    string pw;
#ifdef _WIN32
    char ch;
    while ((ch = _getch()) != '\r' && ch != '\n') {
        if (ch == '\b') { if (!pw.empty()) { pw.pop_back(); cout << "\b \b"; } }
        else { pw += ch; cout << '*'; }
    }
    cout << "\n";
#else
    termios oldt{};
    bool tty = isatty(STDIN_FILENO);
    if (tty) {
        tcgetattr(STDIN_FILENO, &oldt);
        termios newt = oldt;
        newt.c_lflag &= ~static_cast<tcflag_t>(ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    }
    getline(cin, pw);
    if (tty) { tcsetattr(STDIN_FILENO, TCSANOW, &oldt); cout << "\n"; }
#endif
    return pw;
}

// ===================== Validation =====================
bool validateUsername(const string &u, string &err) {
    if (u.size() < 3 || u.size() > 20) { err = "Username must be 3-20 characters long."; return false; }
    for (char c : u)
        if (!isalnum(static_cast<unsigned char>(c)) && c != '_') {
            err = "Username may only contain letters, digits and underscores.";
            return false;
        }
    return true;
}

bool validatePassword(const string &p, const string &u, string &err) {
    if (p.size() < 8) { err = "Password must be at least 8 characters long."; return false; }
    bool hasLetter = false, hasDigit = false;
    for (char c : p) {
        if (isalpha(static_cast<unsigned char>(c))) hasLetter = true;
        if (isdigit(static_cast<unsigned char>(c))) hasDigit = true;
        if (isspace(static_cast<unsigned char>(c))) { err = "Password must not contain spaces."; return false; }
    }
    if (!hasLetter || !hasDigit) { err = "Password must contain at least one letter and one digit."; return false; }
    if (toLower(p) == toLower(u)) { err = "Password must not be the same as the username."; return false; }
    return true;
}

// ===================== Storage =====================
struct Record { string username, salt, hash; };

bool findUser(const string &username, Record &out) {
    ifstream in(DB_FILE);
    if (!in) return false;
    Record r;
    while (in >> r.username >> r.salt >> r.hash) {
        if (toLower(r.username) == toLower(username)) { out = r; return true; }
    }
    return false;
}

bool saveUser(const Record &r) {
    ofstream out(DB_FILE, ios::app);
    if (!out) return false;
    out << r.username << ' ' << r.salt << ' ' << r.hash << '\n';
    return static_cast<bool>(out);
}

// ===================== Features =====================
void registerUser() {
    cout << "\n--- Registration ---\n";
    string username, err;
    cout << "Choose a username: ";
    getline(cin, username);

    if (!validateUsername(username, err)) { cout << "[Error] " << err << "\n"; return; }
    Record existing;
    if (findUser(username, existing)) { cout << "[Error] Username already taken. Try another one.\n"; return; }

    string pw = readPassword("Choose a password: ");
    if (!validatePassword(pw, username, err)) { cout << "[Error] " << err << "\n"; return; }
    string confirm = readPassword("Confirm password: ");
    if (pw != confirm) { cout << "[Error] Passwords do not match.\n"; return; }

    Record r;
    r.username = username;
    r.salt = generateSalt();
    r.hash = hashPassword(pw, r.salt);
    if (saveUser(r)) cout << "[Success] Registration complete. You can now log in.\n";
    else cout << "[Error] Could not write to " << DB_FILE << ".\n";
}

void loginUser() {
    cout << "\n--- Login ---\n";
    string username;
    cout << "Username: ";
    getline(cin, username);
    string pw = readPassword("Password: ");

    Record r;
    // Same generic message for unknown user and wrong password (no user enumeration)
    if (!findUser(username, r) || !constantTimeEquals(hashPassword(pw, r.salt), r.hash)) {
        cout << "[Error] Invalid username or password.\n";
        return;
    }
    cout << "[Success] Login successful. Welcome, " << r.username << "!\n";
}

int main() {
    while (true) {
        cout << "\n==============================\n"
             << "  LOGIN & REGISTRATION SYSTEM\n"
             << "==============================\n"
             << "1. Register\n2. Login\n3. Exit\nChoice: ";
        string choice;
        if (!getline(cin, choice)) break;
        if (choice == "1") registerUser();
        else if (choice == "2") loginUser();
        else if (choice == "3") { cout << "Goodbye!\n"; break; }
        else cout << "[Error] Invalid choice. Enter 1, 2 or 3.\n";
    }
    return 0;
}
