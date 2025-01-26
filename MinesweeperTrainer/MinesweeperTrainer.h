

//0000 - Base              10056C8 : 1
//0004 - Player 1 score    10056CC : 100
//0008 - Player 2 score    10056D0 : 200
//000C - Player 3 score    10056D4 : 300
//0010 - Player 1 name     10056D8 : Hax1
//0050 - Player 2 name     1005718 : Hax2
//0090 - Player 3 name     1005758 : Hax3
//00D4 - Timer             100579C : 170

struct Player_offsetsFrom_WINMINE {
	uintptr_t WINMINE_ModuleAddress;													//0x01000000

	uintptr_t getPlayersBaseOffset() { return WINMINE_ModuleAddress + 0x000056C8; }		//0x010056C8
	uintptr_t getPlayer1score_offset() { return getPlayersBaseOffset() + 0x00000004; }
	uintptr_t getPlayer2score_offset() { return getPlayersBaseOffset() + 0x00000008; }
	uintptr_t getPlayer3score_offset() { return getPlayersBaseOffset() + 0x0000000C; }
	uintptr_t getPlayer1name_offset() { return getPlayersBaseOffset() + 0x00000010; }
	uintptr_t getPlayer2name_offset() { return getPlayersBaseOffset() + 0x00000050; }
	uintptr_t getPlayer3name_offset() { return getPlayersBaseOffset() + 0x00000090; }
	uintptr_t getTimer_offset() { return getPlayersBaseOffset() + 0x000000D4; }

	uintptr_t getMineFieldOffset() { return WINMINE_ModuleAddress + 0x00005360; }//0x00005360	//0x00005361	//0x0000534B
	uintptr_t getMineFieldHeightOffset() { return WINMINE_ModuleAddress + 0x00005338; }
	uintptr_t getMineFieldWidthOffset() { return WINMINE_ModuleAddress + 0x00005334; }
};
struct Player_struct {
	Player_offsetsFrom_WINMINE OFFSETS;

	uintptr_t Player1score;
	uintptr_t Player2score;
	uintptr_t Player3score;
	char Player1name[128];
	char Player2name[128];
	char Player3name[128];
	uintptr_t Timer;

	std::vector<BYTE> MineField2;
	BYTE MineField[2048];	//766	//1024	// 33x33	24x30	(24 + 2) * 30=780
	uintptr_t MineFieldHeight;
	uintptr_t MineFieldWidth;
};

std::string GetProcessName(HANDLE hProcess);

std::vector<DWORD> GetAllProcessesListVector();

uintptr_t GetProcessIDbyProcessName(std::string targetProcessName);

BYTE* GetModuleBaseAddress(DWORD processId, std::string moduleName);

void GetPlayerStructData(HANDLE hProcess, Player_struct& playerStruct);

void printWideString(const char* buffer);

void DrawPointToThisSquareField(Player_struct playerStruct, int row, int col);

void PrintMineFiled(Player_struct playerStruct);

void PrintPlayerStructData(Player_struct playerStruct);

void MinesweeperSolver(Player_struct& playerStruct);

void MinesweeperSolverWriter(HANDLE hProcess, Player_struct& playerStruct);

int main();
