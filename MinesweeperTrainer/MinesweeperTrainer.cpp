#include <windows.h>
#include <iostream>
#include <vector>
#include <Psapi.h>
#include <TlHelp32.h>
#include "MinesweeperTrainer.h"
#include <iomanip>
#include "DrawOverlayTrainer.h"


std::wstring GetProcessName(HANDLE hProcess) {
	TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

	if (hProcess != NULL) {
		HMODULE hMod;
		DWORD cbNeeded;

		if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeeded)) {
			GetModuleBaseName(hProcess, hMod, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));
		}
	}
	return std::wstring(szProcessName);
}
std::vector<DWORD> GetAllProcessesListVector() {
	DWORD aProcesses[1024], cbNeeded, cProcesses;
	std::vector<DWORD> processIds;

	if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded)) {
		return processIds;
	}

	cProcesses = cbNeeded / sizeof(DWORD);

	for (unsigned int i = 0; i < cProcesses; i++) {
		if (aProcesses[i] != 0) {
			processIds.push_back(aProcesses[i]);
			//PrintProcessNameAndID(aProcesses[i]);
		}
	}
	return processIds;
}
uintptr_t GetProcessIDbyProcessName(std::wstring targetProcessName) {
	uintptr_t PID = 0;

	std::vector<DWORD> processList = GetAllProcessesListVector();
	for (int i = 0; i < processList.size(); ++i) {
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processList[i]);
		std::wstring processName = GetProcessName(hProcess);
		if (processName == targetProcessName) {
			//std::cout << Wstring_to_StdString(processName) << std::endl;
			PID = GetProcessId(hProcess);
			CloseHandle(hProcess);
			std::cout << std::dec << PID << std::endl;
			return PID;
		}
		CloseHandle(hProcess);
	}
	//System::Windows::Forms::MessageBox::Show("Can't find process with that PID.");
	return PID;
}
BYTE* GetModuleBaseAddress(DWORD processId, std::wstring moduleName) {
	HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, processId);		// --> TH32CS_SNAPMODULE
	if (hProcessSnap == INVALID_HANDLE_VALUE) {
		return 0;
	}

	MODULEENTRY32 me32 = { 0 };
	me32.dwSize = sizeof(MODULEENTRY32);

	if (Module32FirstW(hProcessSnap, &me32)) {
		do {
			if (me32.szModule == moduleName) {
				CloseHandle(hProcessSnap);
				BYTE* baseAddress = me32.modBaseAddr;
				return baseAddress;
				//return (DWORD)me32.modBaseAddr;
			}
		} while (Module32NextW(hProcessSnap, &me32));
	}
	CloseHandle(hProcessSnap);
	return 0;
}

void GetPlayerStructData(HANDLE hProcess, Player_struct& playerStruct) {

	ReadProcessMemory(hProcess, (LPCVOID)(playerStruct.OFFSETS.getPlayer1score_offset()),
		&playerStruct.Player1score, sizeof(playerStruct.Player1score), NULL);
	ReadProcessMemory(hProcess, (LPCVOID)(playerStruct.OFFSETS.getPlayer1name_offset()),
		playerStruct.Player1name, sizeof(playerStruct.Player2name), NULL);

	ReadProcessMemory(hProcess, (LPCVOID)(playerStruct.OFFSETS.getPlayer2score_offset()),
		&playerStruct.Player2score, sizeof(playerStruct.Player2score), NULL);
	ReadProcessMemory(hProcess, (LPCVOID)(playerStruct.OFFSETS.getPlayer2name_offset()),
		playerStruct.Player2name, sizeof(playerStruct.Player2name), NULL);

	ReadProcessMemory(hProcess, (LPCVOID)(playerStruct.OFFSETS.getPlayer3score_offset()),
		&playerStruct.Player3score, sizeof(playerStruct.Player3score), NULL);
	ReadProcessMemory(hProcess, (LPCVOID)(playerStruct.OFFSETS.getPlayer3name_offset()),
		playerStruct.Player3name, sizeof(playerStruct.Player3name), NULL);

	ReadProcessMemory(hProcess, (LPCVOID)(playerStruct.OFFSETS.getTimer_offset()),
		&playerStruct.Timer, sizeof(playerStruct.Timer), NULL);

	ReadProcessMemory(hProcess, (LPCVOID)(playerStruct.OFFSETS.getMineFieldOffset()),
		&playerStruct.MineField, sizeof(playerStruct.MineField), NULL);
	auto aa = playerStruct.OFFSETS.getMineFieldOffset();
	ReadProcessMemory(hProcess, (LPCVOID)(playerStruct.OFFSETS.getMineFieldHeightOffset()),
		&playerStruct.MineFieldHeight, sizeof(playerStruct.MineFieldHeight), NULL);
	ReadProcessMemory(hProcess, (LPCVOID)(playerStruct.OFFSETS.getMineFieldWidthOffset()),
		&playerStruct.MineFieldWidth, sizeof(playerStruct.MineFieldWidth), NULL);

}
void printWideString(const char* buffer) {
	const wchar_t* wideBuffer = reinterpret_cast<const wchar_t*>(buffer);			// Cast the char* to wchar_t*
	std::wcout << wideBuffer;														// Print the wide character string
}
/// DRAW WITH GDI32.dll
void DrawPointToThisSquareField(Player_struct playerStruct, int row, int col) {
	HWND hWnd = FindWindow(NULL, L"Minesweeper");	// Get Window (form)	
	HDC hDC = GetDC(hWnd);// Get the device context

	//int blockSize = (windowWidth - 35) / 8;		//30	//16
	int blockSize = 16;
	//std::cout << blockSize << std::endl;
	int startX = 20;
	int startY = 63;

	int x = startX + col * blockSize;
	int y = startY + row * blockSize;

	// DRAW BASED ON X AND Y COORDINATES:
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0)); // Set the pen color and thickness
	HGDIOBJ oldPen = SelectObject(hDC, hPen);
	Ellipse(hDC, x - 2, y - 2, x + 2, y + 2);// Draw the dot (filled ellipse)
	SelectObject(hDC, oldPen);// Restore the old pen
	DeleteObject(hPen);
	ReleaseDC(hWnd, hDC);// Release the device context
}
void PrintMineFiled(Player_struct playerStruct) {
	bool allowPrint = false;
	int row = -1;
	int col = 0;
	int endByteSet = 0xEE;
	int desiredLength = 32 * (int)playerStruct.MineFieldHeight;
	playerStruct.MineField[desiredLength] = endByteSet;
	std::cout << (int)playerStruct.MineFieldHeight << " x " << (int)playerStruct.MineFieldWidth << std::endl;
	for (int i = 0; i < (sizeof(playerStruct.MineField)) / 1; ++i) {
		if (playerStruct.MineField[i] == 0x10) {		// TOGGLE ON/OFF -> WHEN NEW LINE IS DETECTED 
			//++i;							// SKIP 0x10 -> DONT PRINT IT -> Works only for smallest field
			if (allowPrint == true) {
				allowPrint = false;
				col++;
			}
			else if (allowPrint == false) {
				allowPrint = true;
				std::cout << std::endl;
				col = 0;
				row++;
			}
		}
		if (allowPrint == true && playerStruct.MineField[i] != 0x10) {
			//DrawPointToThisSquareField(playerStruct, row, col);		// FOR DEBUG -> PRINT DOTS TO ALL SQUARES
			std::cout << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << (int)playerStruct.MineField[i] << " ";// DEFAULT HEX VIEW
			if (playerStruct.MineField[i] == 0x8F) {
				DrawPointToThisSquareField(playerStruct, row, col);
			}
			col++;

			/////////////////////////////////////////////////////////////////// CHANGE CHARACTER SYMBOLS:
			//if (playerStruct.MineField[i] == 0x0F) {
			//	std::cout << "0 ";
			//}
			//else if (playerStruct.MineField[i] == 0x0E) {
			//	std::cout << "F ";
			//}
			//else if (playerStruct.MineField[i] == 0x8D) {
			//	std::cout << "? ";
			//}
			//else if (playerStruct.MineField[i] == 0x8E) {
			//	std::cout << "! ";
			//}
			//else if (playerStruct.MineField[i] == 0x8F) {
			//	std::cout << "X ";
			//}
			//else if (playerStruct.MineField[i] == 0x40
			//	|| playerStruct.MineField[i] == 0x41
			//	|| playerStruct.MineField[i] == 0x42
			//	|| playerStruct.MineField[i] == 0x43
			//	|| playerStruct.MineField[i] == 0x44) {
			//	std::cout << "1 ";
			//}
			//else {
			//	std::cout << "2 ";	// THERE ARE MORE UNDEFINED TYPES
			//}
			///////////////////////////////////////////////////////////////////
		}
		if (playerStruct.MineField[i] == endByteSet) {
			break;
		}
		//std::cout << i ;
	}
	std::cout << std::endl;
}
void PrintPlayerStructData(Player_struct playerStruct) {
	std::cout << std::dec << "Timer: " << playerStruct.Timer << std::endl;

	std::cout << std::dec << "Player 1: ";
	printWideString(playerStruct.Player1name);
	std::cout << "\t\tTop Score: " << playerStruct.Player1score << std::endl;

	std::cout << std::dec << "Player 2: ";
	printWideString(playerStruct.Player2name);
	std::cout << "\t\tTop Score: " << playerStruct.Player2score << std::endl;

	std::cout << std::dec << "Player 3: ";
	printWideString(playerStruct.Player3name);
	std::cout << std::dec << "\t\tTop Score: " << playerStruct.Player3score << std::endl;
}
void MinesweeperSolver(Player_struct& playerStruct) {
	auto mineField = playerStruct.MineField;
	for (int i = 0; i < sizeof(playerStruct.MineField); ++i) {
		if (playerStruct.MineField[i] == 0x8F) {
			playerStruct.MineField[i] = 0x8E;
		}
	}
}
void MinesweeperSolverWriter(HANDLE hProcess, Player_struct& playerStruct) {
	// PROCESS:
	std::wstring processName = L"WINMINE.EXE";
	uintptr_t processId = GetProcessIDbyProcessName(processName);

	// PROCESS:
	HANDLE hProcessWrite = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);		// Get new hangle with ALL ACCESS
	if (hProcess == NULL) {
		std::cerr << "Failed to open process" << std::endl;
		//return 1;
	}
	bool result = WriteProcessMemory(hProcessWrite, (LPVOID)(playerStruct.OFFSETS.getMineFieldOffset()),
		&playerStruct.MineField, sizeof(playerStruct.MineField), NULL);
	if (result) {
		std::cout << "Solver Enabled" << std::endl;
	}
	else {
		std::cout << GetLastError() << std::endl;
	}
	CloseHandle(hProcessWrite);
}
int main() {
	while (true) {
		// PROCESS:
		std::wstring processName = L"WINMINE.EXE";
		uintptr_t processId = GetProcessIDbyProcessName(processName);

		// PROCESS:
		HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, processId);
		if (hProcess == NULL) {
			std::cerr << "Failed to open process" << std::endl;
			//return 1;
		}
		// MODULE:	(Also works)
		std::wstring moduleName = L"WINMINE.EXE";
		BYTE* moduleBaseAddress_b = GetModuleBaseAddress(processId, moduleName);
		uintptr_t moduleBaseAddress = (uintptr_t)moduleBaseAddress_b;
		//std::cout << "Module: 0x" << std::hex << moduleBaseAddress << std::endl;
		if (moduleBaseAddress == 0) {
			std::cerr << "Module not found" << std::endl;
			//return 1;
		}

		Player_struct playerStruct = {};
		playerStruct.OFFSETS.WINMINE_ModuleAddress = moduleBaseAddress;
		GetPlayerStructData(hProcess, playerStruct);
		PrintPlayerStructData(playerStruct);
		PrintMineFiled(playerStruct);

		// SOLVER:
		//MinesweeperSolver(playerStruct);
		//MinesweeperSolverWriter(hProcess, playerStruct);

		CloseHandle(hProcess);

		Sleep(500);
	}
}