#include <iostream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <limits>

using namespace std;

// ==================== CONFIGURABLE CONSTANTS ====================
constexpr double DEFAULT_SAVINGS_RATE = 3.5;

// ==================== ENUMS ====================
enum RequestType { ACCOUNT_CREATION, PROFILE_UPDATE };
enum RequestStatus { PENDING, APPROVED, REJECTED };
enum AccountType { SAVINGS };

// ==================== FORWARD DECLARATION ====================
int getValidatedInt(const string& prompt);

// ==================== USER BASE CLASS ====================
class User {
protected:
    string username, password, fullName;
public:
    User(string un, string pw, string fn) : username(un), password(pw), fullName(fn) {}
    virtual ~User() = default;
    virtual string getRole() const = 0;
    string getUsername() const { return username; }
    string getFullName() const { return fullName; }
    void setFullName(string name) { fullName = name; }
    bool verifyPassword(string pwd) const { return password == pwd; }
    void changePassword(string pwd) { password = pwd; }
};

// ==================== MANAGER & CUSTOMER CLASSES ====================
class Manager : public User {
public:
    Manager(string un, string pw, string fn) : User(un, pw, fn) {}
    string getRole() const override { return "Manager"; }
};

class Customer : public User {
private:
    int accountNumber;
public:
    Customer(string un, string pw, string fn, int acc) : User(un, pw, fn), accountNumber(acc) {}
    string getRole() const override { return "Customer"; }
    int getAccountNumber() const { return accountNumber; }
};

// ==================== TRANSACTION STRUCT ====================
struct Transaction {
    string type; double amount, balanceAfter; time_t timestamp;
    Transaction(string t, double a, double bal)
        : type(t), amount(a), balanceAfter(bal), timestamp(time(nullptr)) {}
};

// ==================== REQUEST CLASS ====================
class Request {
private:
    static inline int nextRequestId = 0;
    int requestId; RequestType type; RequestStatus status;
    // Data fields
    string customerName, username, password, newName;
    double initialBalance, rateOrLimit = DEFAULT_SAVINGS_RATE;
    int accountNumber;
public:
    Request(string cn, string un, string pw, double bal)
        : customerName(cn), username(un), password(pw), initialBalance(bal) {
        requestId = ++nextRequestId; type = ACCOUNT_CREATION; status = PENDING;
    }
    Request(int acc, string nn) : accountNumber(acc), newName(nn) {
        requestId = ++nextRequestId; type = PROFILE_UPDATE; status = PENDING;
    }
    int getRequestId() const { return requestId; }
    RequestType getType() const { return type; }
    RequestStatus getStatus() const { return status; }
    void setStatus(RequestStatus s) { status = s; }
    string getCustomerName() const { return customerName; }
    string getUsername() const { return username; }
    string getPassword() const { return password; }
    double getInitialBalance() const { return initialBalance; }
    double getRateOrLimit() const { return rateOrLimit; }
    int getAccountNumber() const { return accountNumber; }
    string getNewName() const { return newName; }
    void display() const {
        cout << "\n----------------------------------------" << endl;
        cout << "Request ID: " << requestId << endl;
        cout << "Status: " << (status == PENDING ? "PENDING" : (status == APPROVED ? "APPROVED" : "REJECTED")) << endl;
        if (type == ACCOUNT_CREATION) {
            cout << "Type: New Customer Account (Savings)" << endl;
            cout << "Customer: " << customerName << endl;
            cout << "Username: " << username << endl;
            cout << "Initial Deposit: $" << fixed << setprecision(2) << initialBalance << endl;
        } else {
            cout << "Type: Profile Update" << endl;
            cout << "Account: " << accountNumber << endl;
            cout << "New Name: " << newName << endl;
        }
        cout << "----------------------------------------" << endl;
    }
};

// ==================== ACCOUNT BASE CLASS ====================
class Account {
private:
    static inline int nextAccountNumber = 1000;
protected:
    int accountNumber; string accountHolderName; double balance;
    vector<Transaction> transactions;
public:
    Account(string name, double initialBalance)
        : accountHolderName(name), balance(initialBalance) {
        accountNumber = ++nextAccountNumber;
    }
    virtual ~Account() = default;
    virtual string getAccountType() const = 0;
    virtual void deposit(double amount) = 0;
    virtual void withdraw(double amount) = 0;
    virtual void display() const = 0;
    int getAccountNumber() const { return accountNumber; }
    string getAccountHolderName() const { return accountHolderName; }
    double getBalance() const { return balance; }
    void setAccountHolderName(string name) { accountHolderName = name; }
    void setBalance(double bal) { balance = bal; }
    const vector<Transaction>& getTransactionHistory() const { return transactions; }
    void recordTransaction(const string& type, double amount) {
        transactions.emplace_back(type, amount, balance);
    }
};

// ==================== SAVINGS ACCOUNT ====================
class SavingsAccount : public Account {
private:
    double interestRate;
public:
    SavingsAccount(string name, double initialBalance, double rate = DEFAULT_SAVINGS_RATE)
        : Account(name, initialBalance), interestRate(rate) {}
    string getAccountType() const override { return "Savings"; }
    void deposit(double amount) override {
        if (amount > 0) {
            setBalance(getBalance() + amount);
            recordTransaction("DEPOSIT", amount);
            cout << "\n[+] Deposit successful! New Balance: $" << fixed << setprecision(2) << getBalance() << endl;
        } else cout << "\n[-] Invalid deposit amount!" << endl;
    }
    void withdraw(double amount) override {
        if (amount > 0 && amount <= getBalance()) {
            setBalance(getBalance() - amount);
            recordTransaction("WITHDRAWAL", amount);
            cout << "\n[+] Withdrawal successful! New Balance: $" << fixed << setprecision(2) << getBalance() << endl;
        } else cout << "\n[-] Invalid amount or insufficient balance!" << endl;
    }
    void display() const override {
        cout << "\n----------------------------------------" << endl;
        cout << "Account Number: " << getAccountNumber() << endl;
        cout << "Holder Name: " << getAccountHolderName() << endl;
        cout << "Account Type: Savings" << endl;
        cout << "Balance: $" << fixed << setprecision(2) << getBalance() << endl;
        cout << "Interest Rate: " << interestRate << "% p.a." << endl;
        cout << "----------------------------------------" << endl;
    }
};

// ==================== BANK MANAGEMENT SYSTEM ====================
class Bank {
private:
    vector<Account*> accounts;
    vector<User*> users;
    vector<Request*> pendingRequests;
    Account* findAccount(int accNum) const {
        for (auto acc : accounts) if (acc->getAccountNumber() == accNum) return acc;
        return nullptr;
    }
public:
    ~Bank() {
        for (auto acc : accounts) delete acc;
        for (auto user : users) delete user;
        for (auto req : pendingRequests) delete req;
    }
    void initializeManager() { users.push_back(new Manager("admin", "admin123", "Bank Manager")); }
    User* findUser(string username) const {
        for (auto user : users) if (user->getUsername() == username) return user;
        return nullptr;
    }
    Manager* loginManager(string username, string password) {
        User* user = findUser(username);
        Manager* manager = dynamic_cast<Manager*>(user);
        return (manager && manager->verifyPassword(password)) ? manager : nullptr;
    }
    Customer* loginCustomer(string username, string password) {
        User* user = findUser(username);
        Customer* customer = dynamic_cast<Customer*>(user);
        return (customer && customer->verifyPassword(password)) ? customer : nullptr;
    }
    void applyForNewAccount() {
        cout << "\n+--------------------------------------+" << endl;
        cout << "|    APPLY FOR NEW CUSTOMER ACCOUNT    |" << endl;
        cout << "+--------------------------------------+" << endl;
        string name, uname, pwd; double balance;
        
        // FIX: Removed cin.ignore() here. The buffer is already clean
        // because of cin.ignore() after the 'y/n' prompt in main().
        cout << "Enter Full Name: "; 
        getline(cin, name);
        
        cout << "Enter Desired Username: "; cin >> uname;
        cout << "Enter Password: "; cin >> pwd;
        cout << "Enter Initial Deposit Amount: $"; cin >> balance;
        submitAccountRequest(name, uname, pwd, balance);
        cout << "\n[+] Application submitted! Manager approval required." << endl;
    }
    // ==================== REQUEST METHODS ====================
    void submitAccountRequest(string cname, string uname, string pwd, double balance) {
        if (findUser(uname)) { cout << "\n[-] Username exists! Choose another." << endl; return; }
        pendingRequests.push_back(new Request(cname, uname, pwd, balance));
        cout << "\n[+] Request submitted!" << endl;
    }
    void submitProfileRequest(int accNum, string newName) {
        if (!findAccount(accNum)) { cout << "\n[-] Account not found!" << endl; return; }
        pendingRequests.push_back(new Request(accNum, newName));
        cout << "\n[+] Profile update request submitted!" << endl;
    }
    void viewPendingRequests() const {
        bool found = false;
        for (auto req : pendingRequests) if (req->getStatus() == PENDING) {
            if (!found) { cout << "\n------ PENDING REQUESTS ------" << endl; found = true; }
            req->display();
        }
        if (!found) cout << "\n[+] No pending requests!" << endl;
    }
    void processRequest() {
        if (pendingRequests.empty()) { cout << "\n[-] No requests!" << endl; return; }
        viewPendingRequests();
        int reqId = getValidatedInt("\nEnter Request ID (0 to cancel): ");
        if (reqId == 0) return;
        auto it = find_if(pendingRequests.begin(), pendingRequests.end(),
            [reqId](Request* r) { return r->getRequestId() == reqId && r->getStatus() == PENDING; });
        if (it == pendingRequests.end()) { cout << "\n[-] Request not found!" << endl; return; }
        Request* req = *it;
        cout << "\nProcessing Request #" << reqId << ":" << endl;
        req->display();
        char action = ' ';
        while (action != 'a' && action != 'r') {
            cout << "\n(A)pprove or (R)eject? "; cin >> action; action = tolower(action);
        }
        if (action == 'a') {
            req->setStatus(APPROVED);
            if (req->getType() == ACCOUNT_CREATION) {
                Account* acc = new SavingsAccount(req->getCustomerName(), req->getInitialBalance(), req->getRateOrLimit());
                accounts.push_back(acc);
                users.push_back(new Customer(req->getUsername(), req->getPassword(), req->getCustomerName(), acc->getAccountNumber()));
                cout << "\n[+] REQUEST APPROVED! New Customer and Savings Account created!" << endl;
            } else {
                Account* acc = findAccount(req->getAccountNumber());
                acc->setAccountHolderName(req->getNewName());
                cout << "\n[+] REQUEST APPROVED! Profile updated!" << endl;
            }
        } else {
            req->setStatus(REJECTED);
            cout << "\n[-] REQUEST REJECTED!" << endl;
        }
    }
    // ==================== ACCOUNT OPERATIONS ====================
    void displayAllAccounts() const {
        if (accounts.empty()) { cout << "\n[-] No accounts!" << endl; return; }
        cout << "\n------ ALL BANK ACCOUNTS ------" << endl;
        for (auto acc : accounts) acc->display();
    }
    void displayCustomerAccount(int accNum) const {
        Account* acc = findAccount(accNum);
        if (acc) acc->display();
        else cout << "\n[-] Account Not Found!" << endl;
    }
    void displayTransactionHistory(int accNum) const {
        Account* acc = findAccount(accNum);
        if (!acc) { cout << "\n[-] Account Not Found!" << endl; return; }
        const auto& history = acc->getTransactionHistory();
        if (history.empty()) { cout << "\n[i] No transactions." << endl; return; }
        cout << "\n------ TRANSACTION HISTORY ------" << endl;
        cout << "Account #" << accNum << " - " << acc->getAccountHolderName() << endl;
        for (const auto& txn : history) {
            cout << "Type: " << left << setw(14) << txn.type
                 << " | Amount: $" << right << setw(10) << fixed << setprecision(2) << txn.amount
                 << " | Balance: $" << setw(10) << txn.balanceAfter << endl;
        }
    }
    void customerTransaction(int accNum, bool isDeposit) {
        Account* acc = findAccount(accNum);
        if (!acc) { cout << "\n[-] Account Not Found!" << endl; return; }
        double amount;
        cout << "\n------ " << (isDeposit ? "DEPOSIT" : "WITHDRAW") << " MONEY ------" << endl;
        cout << "Enter Amount: $"; cin >> amount;
        isDeposit ? acc->deposit(amount) : acc->withdraw(amount);
    }
    void transferMoney(int fromAcc, int toAcc, double amount) {
        Account* from = findAccount(fromAcc), *to = findAccount(toAcc);
        if (!from || !to) { cout << "\n[-] One or both accounts not found!" << endl; return; }
        if (fromAcc == toAcc || amount <= 0 || amount > from->getBalance()) {
            cout << "\n[-] Invalid transfer!" << endl; return;
        }
        from->setBalance(from->getBalance() - amount);
        to->setBalance(to->getBalance() + amount);
        from->recordTransaction("TRANSFER_OUT", amount);
        to->recordTransaction("TRANSFER_IN", amount);
        cout << "\n[+] Transfer successful!" << endl;
    }
};

// ==================== VALIDATION FUNCTION ====================
int getValidatedInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value && cin.peek() == '\n') return value;
        cout << "\n[-] Invalid input! Please enter a number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// ==================== MENU FUNCTIONS ====================
void showManagerMenu(Bank& bank) {
    int choice;
    do {
        cout << "\n==========================================" << endl;
        cout << "           MANAGER DASHBOARD              " << endl;
        cout << "1. View Pending Requests\n2. Process Request\n3. Display All Accounts\n4. Logout" << endl;
        cout << "==========================================" << endl;
        choice = getValidatedInt("Enter choice (1-4): ");
        switch (choice) {
            case 1: bank.viewPendingRequests(); break;
            case 2: bank.processRequest(); break;
            case 3: bank.displayAllAccounts(); break;
            case 4: cout << "\nLogging out..." << endl; break;
            default: cout << "\n[-] Invalid choice! Select 1-4." << endl;
        }
        if (choice != 4) {
            cout << "\nPress Enter to continue...";
            cin.ignore(); cin.get();
        }
    } while (choice != 4);
}

void showCustomerMenu(Bank& bank, Customer* customer) {
    int choice, accNum = customer->getAccountNumber();
    do {
        cout << "\n==========================================" << endl;
        cout << "   CUSTOMER DASHBOARD (" << customer->getFullName() << ")" << endl;
        cout << "      YOUR ACCOUNT PORTFOLIO     " << endl;
        bank.displayCustomerAccount(accNum);
        cout << "\n------------------ MENU ------------------" << endl;
        cout << "1. View Account\n2. Deposit\n3. Withdraw\n4. Transfer\n5. Transaction History\n6. Update Name\n7. Change Password\n8. Logout" << endl;
        cout << "==========================================" << endl;
        choice = getValidatedInt("Enter choice (1-8): ");
        switch (choice) {
            case 1: break;
            case 2:
            case 3: bank.customerTransaction(accNum, choice == 2); break;
            case 4: {
                cout << "\n------ TRANSFER MONEY ------" << endl;
                int toAcc = getValidatedInt("Enter Destination Account (0 to cancel): ");
                if (toAcc == 0) break;
                double amount; cout << "Enter Amount: $"; cin >> amount;
                bank.transferMoney(accNum, toAcc, amount);
                break;
            }
            case 5: bank.displayTransactionHistory(accNum); break;
            case 6: {
                cout << "\nSubmitting name update for Account #" << accNum << endl;
                string newName; cout << "Enter New Name: "; cin.ignore(); getline(cin, newName);
                bank.submitProfileRequest(accNum, newName);
                break;
            }
            case 7: {
                cout << "\n------ CHANGE PASSWORD ------" << endl;
                string oldPass, newPass;
                cout << "Enter Current Password: "; cin >> oldPass;
                if (!customer->verifyPassword(oldPass))
                    cout << "\n[-] Incorrect password!" << endl;
                else {
                    cout << "Enter New Password: "; cin >> newPass;
                    customer->changePassword(newPass);
                    cout << "\n[+] Password changed!" << endl;
                }
                break;
            }
            case 8: cout << "\nLogging out..." << endl; break;
            default: cout << "\n[-] Invalid choice! Select 1-8." << endl;
        }
        if (choice == 8) break;
        cout << "\nPress Enter to continue...";
        cin.ignore(); cin.get();
    } while (true);
}

// ==================== MAIN ====================
int main() {
    Bank bank; bank.initializeManager();
    cout << "\n+-----------------------------------+" << endl;
    cout << "|  WELCOME TO BANKODE  |" << endl;
    cout << "| BANK MANAGEMENT SYSTEM |" << endl;
    cout << "+-----------------------------------+" << endl;
    while (true) {
        cout << "\n==========================================" << endl;
        cout << "                 MAIN MENU                  " << endl;
        cout << "1. Manager Login (admin/admin123)\n2. Customer Login\n3. Exit" << endl;
        cout << "==========================================" << endl;
        int role = getValidatedInt("Enter choice (1-3): ");
        if (role == 3) { cout << "\nGoodbye!" << endl; break; }
        if (role != 1 && role != 2) { cout << "\n[-] Invalid choice!" << endl; continue; }
        string username, password;
        cout << "\n+--------------------------+" << endl;
        cout << "|         LOGIN            |" << endl;
        cout << "+--------------------------+" << endl;
        cout << "Username: "; cin >> username;
        cout << "Password: "; cin >> password;
        if (role == 1) {
            if (Manager* manager = bank.loginManager(username, password)) {
                cout << "\n[+] Welcome, Manager" << endl;
                showManagerMenu(bank);
            } else cout << "\n[-] Invalid credentials!" << endl;
        } else {
            User* user = bank.findUser(username);
            if (!user) {
                cout << "\n[-] Username not found!" << endl;
                cout << "Apply for new account? (y/n): "; char apply; cin >> apply;
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (apply == 'y' || apply == 'Y') bank.applyForNewAccount();
            } else if (Customer* customer = dynamic_cast<Customer*>(user)) {
                if (customer->verifyPassword(password)) {
                    cout << "\n[+] Welcome, " << customer->getFullName() << endl;
                    showCustomerMenu(bank, customer);
                } else cout << "\n[-] Invalid password!" << endl;
            } else cout << "\n[-] Not a customer account!" << endl;
        }
        cout << "\nPress Enter to return to main menu...";
        cin.ignore(); cin.get();
    }
    return 0;
}