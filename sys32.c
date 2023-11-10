int main(int argc, char** argv) {
	return 9 * 3;
}
void _start(void) {
	char* arg = 0;
	exit(main(0, &arg));
}
