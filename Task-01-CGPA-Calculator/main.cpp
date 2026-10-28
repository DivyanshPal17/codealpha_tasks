// CodeAlpha C++ Internship - Task 1: CGPA Calculator
// Calculates semester GPA and overall CGPA on a 10.0 scale.

#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <vector>
#include <cctype>
#include <map>

using namespace std;

struct Course {
    string name;
    string letter;
    double gradePoint;
    double credits;
};

struct Semester {
    vector<Course> courses;
    double totalCredits = 0;
    double totalPoints = 0;
    double gpa() const { return totalCredits > 0 ? totalPoints / totalCredits : 0.0; }
};

// 10-point scale
const map<string, double> GRADE_SCALE = {
    {"A+", 10.0}, {"A", 9.5}, {"A-", 9.0},
    {"B+", 8.5}, {"B", 8.0}, {"B-", 7.5},
    {"C+", 7.0}, {"C", 6.5}, {"C-", 6.0},
    {"D+", 5.5}, {"D", 5.0}, {"F", 0.0}
};

string toUpper(string s) {
    for (char &c : s) c = static_cast<char>(toupper(static_cast<unsigned char>(c)));
    return s;
}

void clearInput() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int readInt(const string &prompt, int minVal, int maxVal) {
    int v;
    while (true) {
        cout << prompt;
        if (cin >> v && v >= minVal && v <= maxVal) { clearInput(); return v; }
        cout << "  Invalid input. Enter a whole number between " << minVal << " and " << maxVal << ".\n";
        clearInput();
    }
}

double readCredits(const string &prompt) {
    double v;
    while (true) {
        cout << prompt;
        if (cin >> v && v > 0 && v <= 10) { clearInput(); return v; }
        cout << "  Invalid credit hours. Enter a number greater than 0 and up to 10.\n";
        clearInput();
    }
}

// Accepts a letter grade (A, B+, ...) or a numeric grade point (0.0 - 10.0)
double readGrade(const string &prompt, string &letterOut) {
    while (true) {
        cout << prompt;
        string in;
        cin >> in;
        clearInput();
        string up = toUpper(in);
        auto it = GRADE_SCALE.find(up);
        if (it != GRADE_SCALE.end()) { letterOut = up; return it->second; }
        try {
            size_t pos;
            double gp = stod(in, &pos);
            if (pos == in.size() && gp >= 0.0 && gp <= 10.0) {
                letterOut = "-";
                return gp;
            }
        } catch (...) {}
        cout << "  Invalid grade. Use A+, A, A-, B+, B, B-, C+, C, C-, D+, D, F or a number 0.0-10.0.\n";
    }
}

void printSemester(const Semester &s, int number) {
    cout << "\n--- Semester " << number << " Report ---\n";
    cout << left << setw(5) << "No." << setw(22) << "Course" << setw(8) << "Grade"
         << setw(8) << "Points" << setw(9) << "Credits" << "Quality Pts\n";
    cout << string(60, '-') << "\n";
    int i = 1;
    for (const Course &c : s.courses) {
        cout << left << setw(5) << i++ << setw(22) << c.name.substr(0, 20) << setw(8) << c.letter
             << setw(8) << fixed << setprecision(2) << c.gradePoint
             << setw(9) << setprecision(1) << c.credits
             << setprecision(2) << c.gradePoint * c.credits << "\n";
    }
    cout << string(60, '-') << "\n";
    cout << "Total Credits : " << fixed << setprecision(1) << s.totalCredits << "\n";
    cout << "Total Points  : " << setprecision(2) << s.totalPoints << "\n";
    cout << "Semester GPA  : " << setprecision(2) << s.gpa() << "\n";
}

int main() {
    cout << "=====================================\n";
    cout << "          CGPA CALCULATOR\n";
    cout << "=====================================\n";
        cout << "Grade scale: A+=10.0  A=9.5  A-=9.0  B+=8.5  B=8.0  B-=7.5\n"
            << "             C+=7.0  C=6.5  C-=6.0  D+=5.5  D=5.0  F=0.0\n\n";

    int numSemesters = readInt("Number of semesters to enter: ", 1, 12);

    vector<Semester> semesters;
    double cumCredits = 0, cumPoints = 0;

    for (int s = 1; s <= numSemesters; ++s) {
        cout << "\n===== Semester " << s << " =====\n";
        int n = readInt("Number of courses taken: ", 1, 15);
        Semester sem;

        for (int i = 1; i <= n; ++i) {
            Course c;
            cout << "\nCourse " << i << " of " << n << "\n";
            cout << "  Course name: ";
            getline(cin, c.name);
            if (c.name.empty()) c.name = "Course " + to_string(i);
            c.gradePoint = readGrade("  Grade (e.g. A, B+ or 3.5): ", c.letter);
            c.credits = readCredits("  Credit hours: ");
            sem.totalCredits += c.credits;
            sem.totalPoints += c.gradePoint * c.credits;
            sem.courses.push_back(c);
        }

        cumCredits += sem.totalCredits;
        cumPoints += sem.totalPoints;
        semesters.push_back(sem);
    }

    cout << "\n\n=========== FINAL RESULTS ===========\n";
    for (size_t i = 0; i < semesters.size(); ++i) printSemester(semesters[i], static_cast<int>(i) + 1);

    double cgpa = cumCredits > 0 ? cumPoints / cumCredits : 0.0;
    cout << "\n=====================================\n";
    cout << "Total Credits (all semesters): " << fixed << setprecision(1) << cumCredits << "\n";
    cout << "Total Grade Points           : " << setprecision(2) << cumPoints << "\n";
    cout << "Overall CGPA                 : " << setprecision(2) << cgpa << " / 10.00\n";
    cout << "=====================================\n";
    return 0;
}
