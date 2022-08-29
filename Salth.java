import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Arrays;
public class Salth {
	static ArrayList<Var> vars;
	static ArrayList<Svar> vass;
	static ArrayList<String> vans;
	static String cleanse(String s) {
		return cleanse(Integer.decode(s));
	}
	static String cleanse(int i) {
		return "0x" + Integer.toHexString(i);
	}
	public synchronized static void main(String[] args) {
		BufferedReader inr = new BufferedReader(new InputStreamReader(System.in));
		String tin;
		String[] tre;
		vars = new ArrayList<Var>();
		vass = new ArrayList<Svar>();
		vans = new ArrayList<String>();
		int n;
		int p;
		int c;
		int line = 0;
		boolean ahp = false;
		boolean alp = false;
		boolean bhp = false;
		boolean blp = false;
		boolean chp = false;
		boolean clp = false;
		boolean dhp = false;
		boolean dlp = false;
		boolean fuo = false;
		boolean raw = false;
		try {
			System.out.println(".globl _start");
			System.out.println(".text");
			System.out.println(".code16");
			while ((tin = inr.readLine()) != null) {
				line++;
				tre = tin.split(" ");
				if (raw) {
					if (tin.equals("endraw")) {
						raw = false;
						continue;
					}
					System.out.println(tin);
					continue;
				}
				else if (tin.equals("endraw")) {
					throw new Exception("Cannot end raw section outside of a raw section");
				}
				if (tin.equals("startraw")) {
					raw = true;
					continue;
				}
				if (tre[0].startsWith("//")) {
					continue;
				}
				if (tre.length == 0) {
					continue;
				}
				if (tre[0].equals("func")) {
					if (fuo) {
						throw new Exception("Function declaration not allowed within functions");
					}
					n = Integer.decode(tre[2]);
					if ((n > 16) || (n < 0)) {
						throw new Exception ("Argument count (" + Integer.toString(n) + ") is not in the range [0, 16]");
					}
					System.out.println("func_" + tre[1] + "_" + Integer.toString(n) + ":");
					fuo = true;
					continue;
				}
				if (tre[0].equals("lbl")) {
					System.out.println(tre[1] + ":");
					continue;
				}
				if (tre[0].equals("goto")) {
					System.out.println("jmp " + tre[1]);
					continue;
				}
				if (tre[0].equals("return")) {
					solv(Arrays.copyOfRange(tre, 1, tre.length));
					System.out.println("ret");
					continue;
				}
				if (tre[0].equals("endfunc")) {
					if (!fuo) {
						throw new Exception("Function ending not allowed outside of functions");
					}
					System.out.println("ret");
					fuo = false;
					continue;
				}
				if (tre[0].equals("call")) {
					n = 2;
					p = 2;
					c = 0;
					if (tre.length > 2) {
						while (true) {
							while (!(tre[n].equals(","))) {
								n++;
								if (tre.length == n) {
									break;
								}
							}
							solv(Arrays.copyOfRange(tre, p, n));
							System.out.println("movw %ax,%ds:" + cleanse(c * 2));
							if (n == tre.length) {
								break;
							}
							n++;
							p = n;
							c++;
						}
						c++;
						System.out.println("call func_" + tre[1] + "_" + Integer.toString(c));
						continue;
					}
					System.out.println("call func_" + tre[1] + "_0");
					continue;
				}
				if (tre[0].equals("decl")) {
					vars.add((new Var()).name(tre[1]).pos(n = (Var.curp = Var.curp + 2)));
					vans.add(tre[1]);
					solv(Arrays.copyOfRange(tre, 2, tre.length));
					System.out.println("movw %ax,%ds:" + cleanse(n));
					continue;
				}
				if ((!vans.contains(tre[0])) && (!tre[0].startsWith("arg"))) {
					throw new Exception("Undefined variable \"" + tre[0] + "\"");
				}
				if (tre[0].equals("=")) {
					throw new Exception("Illegal operator: Expected '='");
				}
				sols(tre);
			}
			System.out.println("RESstrstart:");
			for(Svar vs : vass) {
				System.out.println("str_" + vs.name + ':');
				System.out.println(".asciz \"" + vs.cont + "\"");
				System.out.println(".set str_" + vs.name + "_len, ( . - str_" + vs.name + " - 1 )");
				System.out.println(".set str_" + vs.name + "_addr, ( str_" + vs.name + " - RESstrstart )");
			}
		}
		catch (Exception E) {
			System.out.println("Line " + line + ": " + E);
			E.printStackTrace();
		}
	}
	static void sols(String[] ss) throws Exception {
		solv(Arrays.copyOfRange(ss, 2, ss.length));
		System.out.println("movw %ax,%ds:" + cleanse(locv(ss[0])));
	}
	static void solv(String[] ss) throws Exception {
		if (ss.length == 0) {
			return;
		}
		int dep = 0;
		int mdep = 0;
		for (String s : ss) {
			if (s.equals("(")) {
				dep++;
				mdep = ((dep > mdep) ? dep : mdep);
			}
			else if (s.equals(")")) {
				dep--;
			}
		}
		if (dep != 0) {
			throw new Exception("Unbalanced parentheses");
		}
		if (mdep == 0) {
			int j;
			bring(ss[0], "ax");
			for (j = 0; j < (ss.length - 1); j += 2) {
				bring(ss[j + 2], "bx");
				if (ss[j + 1].length() != 1) {
					throw new Exception("Invalid infix operator: \"" + ss[j + 1] + "\"");
				}
				switch (ss[j + 1].charAt(0)) {// TODO Avoid loading immediates into %bx
					case ('+'):
						System.out.println("addw %bx,%ax");
						break;
					case ('-'):
						System.out.println("subw %bx,%ax");
						break;
					case ('/'):
						System.out.println("divb %bl");
						System.out.println("xorb %ah,%ah");
						break;
					case ('%'):
						System.out.println("divb %bl");
						System.out.println("movb %ah,%al");
						System.out.println("xorb %ah,%ah");
						break;
					case ('*'):
						System.out.println("mulb %bl");
						break;
					case ('^'):
						System.out.println("xorw %bx,%ax");
						break;
					case ('&'):
						System.out.println("andw %bx,%ax");
						break;
					case ('|'):
						System.out.println("orw %bx,%ax");
						break;
					default:
						throw new Exception("Invalid infix operator: \"" + ss[j] + "\"");
				}
			}
			return;
		}
		throw new Exception("not implemented");
	}
	static void bring(String vel, String reg) throws Exception {
		try {
			System.out.println("movw $" + cleanse(vel) + ",%" + reg);
		}
		catch (NumberFormatException E) {
			System.out.println("movw %ds:" + cleanse(locv(vel)) + ",%" + reg);
		}
	}
	static int locv(String n) throws Exception {
		if (n.startsWith("arg")) {
			return Integer.decode(n.substring(3)) * 2;
		}
		int i = vans.indexOf(n);
		if (i == (-1)) {
			throw new Exception("Undefined variable \"" + n + "\"");
		}
		return vars.get(i).pos;
	}
}
class Var {
	static int curp = 30;
	String name;
	int pos;
	Var() {
		name = null;
		pos = 0;
	}
	Var name(String h) {
		name = h;
		return this;
	}
	Var pos(int h) {
		pos = h;
		return this;
	}
	public boolean equals(Object h) {
		if (h instanceof String) {
			return h.equals(name);
		}
		return name.equals(((Var) h).name);
	}
}
class Svar {
	String name;
	String cont;// Is not yet escaped
	Svar() {
		name = null;
		cont = null;
	}
	Svar name(String s) {
		name = s;
		return this;
	}
	Svar cont(String s) {
		cont = s;
		return this;
	}
	public boolean equals (Object n) {
		return name.equals(((Svar) n).name);
	}
}
