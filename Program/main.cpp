// Lab_9_3
#include <iostream>
#include <iomanip>
#include <fstream>
#include <windows.h>
#include <string>
#include <algorithm>
using namespace std;

// Console 

namespace cons {

	COORD GetBufferSize() {
		static HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbInfo;
		GetConsoleScreenBufferInfo(handle, &csbInfo);
		return { csbInfo.srWindow.Right - csbInfo.srWindow.Left ,
				csbInfo.srWindow.Bottom - csbInfo.srWindow.Top };
	}

	const COORD size = GetBufferSize();

	void clear() {
		system("cls");
	}

	void gotoxy(const COORD pos) {
		static HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleCursorPosition(handle, pos);
	}

	COORD getxy() {
		static HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!GetConsoleScreenBufferInfo(handle, &csbi))
			return { -1, -1 };
		return csbi.dwCursorPosition;
	}

	void clearto(const COORD pos) {
		COORD current_pos = getxy();
		while (current_pos.Y >= pos.Y)
		{
			if (current_pos.Y > pos.Y) {
				gotoxy({ 0, current_pos.Y });
				for (int i = 0; i < size.X; i++)
					cout << ' ';
			}
			else if (current_pos.Y == pos.Y) {
				gotoxy({ pos.X, current_pos.Y });
				for (int i = 0; i <= size.X - pos.X; i++)
					cout << ' ';
			}

			current_pos.Y--;
		}
		gotoxy(pos);
	}

	void change_cusor_visibility(const bool& rst) {
		static HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_CURSOR_INFO structCursorInfo;
		GetConsoleCursorInfo(handle, &structCursorInfo);
		structCursorInfo.bVisible = rst;
		SetConsoleCursorInfo(handle, &structCursorInfo);
	}

	template <typename T>
	T input(bool (*check)(T& temp, char* err), const bool& rom, const char* text, ...) {
		COORD start[2] = { getxy() };
		char out[256] = { 0 }, err[256] = { 0 };
		T temp;

		va_list  args;
		va_start(args, text);
		vsprintf_s(out, 255, text, args);
		va_end(args);
		cout << out;
		start[1] = getxy();
		if (check == nullptr)
			check = [](T& temp, char* err) -> bool { return !cin.fail(); };
		do {
			cin.clear();
			cin.ignore();
			if (err[0] != '\0') {
				change_cusor_visibility(false);
				clearto(start[0]);
				cout << err << endl;
				err[0] = '\0';
				cout << out;
				start[1] = getxy();
				change_cusor_visibility(true);
			}
			else clearto(start[1]);
			cin >> temp;
		} while (!check(temp, err));
		if (rom)
			clearto(start[0]);
		return temp;
	}
}

struct Bill {
	string r_r_payer;
	string r_r_recipient;
	float sum;
};

Bill* InitBill() {
	cout << "Adding new bill: " << endl;
	Bill* bill = new Bill();
	bill->r_r_payer = cons::input<string>(nullptr, false, "Input payer IBAN: ");
	bill->r_r_recipient = cons::input<string>(nullptr, false, "Input recipient IBAN: ");
	bill->sum = cons::input<float>([](float &temp, char *err)-> bool {
		if (cin.fail() ||
			temp <= 0)
		{
			sprintf_s(err, 255, "Incorrect money (money > 0) (%f)", temp);
			return false;
		}
		return true;
	}, false, "Input transfered money: ");
	return bill;
}

void sort_bill(Bill** bills, const unsigned int& size) {
	sort(bills, bills + size, [](Bill* a1, Bill* a2) -> bool {
		return a1->r_r_payer < a2->r_r_payer;
	});
}

int get_bill_by_payer(Bill** bills, const unsigned int& size, const string& payer) {
	for (int i = 0; i < size; i++)
		if (bills[i]->r_r_payer == payer)
			return i;
	return -1;
}

void draw_bills(Bill** bills, const unsigned int& size) {
	cout << "============================================================================================" << endl;
	cout << "| Id | Payer IBAN                         | Recipient IBAN                     | Sum       |" << endl;
	cout << "--------------------------------------------------------------------------------------------" << endl;
	for (int i = 0; i < size; i++) {
		cout << "| " << setw(3) << i + 1
			<< "| " << setw(35) << bills[i]->r_r_payer
			<< "| " << setw(35) << bills[i]->r_r_recipient
			<< "| " << setw(10) << fixed << setprecision(2) << bills[i]->sum
			<< "|" << endl;
	}
	cout << "============================================================================================" << endl;
}

void AddBill(Bill**& bills, int& count, Bill *new_bill) {
	Bill** n_bills = new Bill * [++count];
	memcpy(n_bills, bills, (count - 1) * sizeof(Bill*));
	delete[] bills;
	bills = n_bills;
	bills[count - 1] = new_bill;
}

void save_string_to_file(ofstream& file, const string& str) {
	const int size = str.size();
	file.write(reinterpret_cast<const char*>(&size), sizeof(int));
	file.write(str.c_str(), size);
}

void save_bill_to_file(ofstream& file, Bill *bill) {
	save_string_to_file(file, bill->r_r_payer);
	save_string_to_file(file, bill->r_r_recipient);
	file.write(reinterpret_cast<const char*>(&bill->sum), sizeof(float));
}

void save_bills_to_file(ofstream& file, Bill** bills, const int &size) {
	file.write(reinterpret_cast<const char*>(&size), sizeof(int));
	for (int i = 0; i < size; i++)
		save_bill_to_file(file, bills[i]);
}

string read_string_from_file(ifstream& file) {
	int size = 0;
	file.read(reinterpret_cast<char*>(&size), sizeof(int));
	char* temp = new char[size + 1];
	file.read(reinterpret_cast<char*>(temp), size);
	temp[size] = '\0';
	string str(temp);
	delete[] temp;
	return str;
}

Bill* read_bill_from_file(ifstream& file) {
	Bill* bill = new Bill();
	bill->r_r_payer = read_string_from_file(file);
	bill->r_r_recipient = read_string_from_file(file);
	file.read(reinterpret_cast<char*>(&bill->sum), sizeof(float));
	return bill;
}

Bill** read_bills_from_file(ifstream& file, int& size) {
	file.read(reinterpret_cast<char*>(&size), sizeof(int));
	Bill** bills = new Bill * [size];
	for (int i = 0; i < size; i++)
		bills[i] = read_bill_from_file(file);
	return bills;
}

void main() {
	SetConsoleCP(1251); // встановлення сторінки win-cp1251 в потік вводу
	SetConsoleOutputCP(1251); // встановлення сторінки win-cp1251 в потік виводу
	cout.setf(ios_base::left);

	Bill** bills = nullptr;
	int count = 0, state = 1;
	char message[256] = { 0 };
	do {
		cons::clear();
		//cout << bin_search_student(st, count, "abcd max", 2, 3) << endl;
		draw_bills(bills, count);
		if (message[0] != '\0') {
			cout << message << endl;
			message[0] = '\0';
		}
		cout << "[1] Add bill" << endl
			<< "[2] Sort bill by sender IBAN" << endl
			<< "[3] Search bill by payer" << endl
			<< "[4] Save bills to file" << endl 
			<< "[5] Load bills from file" << endl
			<< "[0] Exit" << endl;
		state = cons::input<unsigned int>(nullptr, false, "Select action: ");
		switch (state)
		{
		case 1:
			AddBill(bills, count, InitBill());
			sprintf_s(message, 255, "Succesful add bill!");
			break;
		case 2:
			sort_bill(bills, count);
			sprintf_s(message, 255, "Succesful sort bills!");
			break;
		case 3: {
			int id;
			string payer = cons::input<string>(nullptr, false, "Input payer IBAN: ");
				if (id = get_bill_by_payer(bills, count, payer), id != -1)
					sprintf_s(message, 255, "Succesful search bill - %s with sum %f!", bills[id]->r_r_payer.c_str(), bills[id]->sum);
				else sprintf_s(message, 255, "Incorrect bill payer - %s!", payer.c_str());
			}
			break;
		case 4:{
			string file_name = cons::input<string>([](string& temp, char* err) -> bool {
				if (cin.fail() ||
					temp.size() < 1) {
					strcpy_s(err, 255, "Incorrect file name!");
					return false;
				}
				return true;
			}, false, "Input filename: ");
			ofstream file = ofstream(file_name, std::ios_base::binary);
			if (file.is_open()) {
				save_bills_to_file(file, bills, count);
				file.close();
				sprintf_s(message, 255, "Succesful save bills to file %s", file_name.c_str());
			}else sprintf_s(message, 255, "Can't open file %s!", file_name.c_str());
		}
			  break;
		case 5: {
			string file_name = cons::input<string>([](string& temp, char* err) -> bool {
				if (cin.fail() ||
					temp.size() < 1) {
					strcpy_s(err, 255, "Incorrect file name!");
					return false;
				}
				return true;
				}, false, "Input filename: ");
			ifstream file = ifstream(file_name, std::ios_base::binary);
			if (file.is_open()) {
				if (bills != nullptr) {
					for (int i = 0; i < count; i++)
						delete bills[i];
					delete[] bills;
					count = 0;
				}
				bills = read_bills_from_file(file, count);
				file.close();
				sprintf_s(message, 255, "Succesful load bills from file %s", file_name.c_str());
			}
			else sprintf_s(message, 255, "Can't open file %s!", file_name.c_str());
		}
		break;
		default:
			sprintf_s(message, 255, "Incorrect action!");
		}
	} while (state > 0);

	if (bills != nullptr) {
		for (int i = 0; i < count; i++)
			delete bills[i];
		delete[] bills;
		count = 0;
	}
}