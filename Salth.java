import java.io.BufferedReader;
import java.io.InputStreamReader;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.EmptyStackException;
import java.util.Stack;
public class Salth {
	static ArrayList<Var> vars;
	static ArrayList<Svar> vass;
	static ArrayList<String> vans;
	static ArrayList<String> vees;
	static Stack<Integer> blocks;
	static Stack<Integer> blid;
	static int blex;
	static int blo;
	static int brer;
	static String pref;
	static String cleanse(String s) {
		return cleanse(Integer.decode(s));
	}
	static String cleanse(int i) {// TODO Warn if out of 16-bit integer limit or 8-bit integer limit and pad / truncate hex (as appropriate)
		return "0x" + Integer.toHexString(i);
	}
	public synchronized static void main(String[] args) {// TODO Implement syntax for static linking from other source files, only import used functions
		BufferedReader inr = new BufferedReader(new InputStreamReader(System.in));
		String tin;
		String[] tre;
		vars = new ArrayList<Var>();
		vass = new ArrayList<Svar>();
		vans = new ArrayList<String>();
		vees = new ArrayList<String>();
		blocks = new Stack<Integer>();
		blid = new Stack<Integer>();
		blex = 0;
		blo = 0;
		brer = 0;
		int n;
		int p;
		int c;
		int line = 0;
		int reer;
		boolean fuo = false;
		boolean raw = false;
		try {
			pref = "";
			if (args.length > 0) {
				if (args.length > 1) {
					pref = args[1] + "_";
				}
				if (args[0].contains("s")) {
					System.out.println(".globl " + pref + "_start");
					System.out.println(".text");
					System.out.println(".code16");
				}
				System.out.println("_" + pref + "start:");
				if (!(args[0].contains("u"))) {
					System.out.println("movw %cs,%bx");
					System.out.println("addw $" + pref + "RESrmstroff,%bx");
					System.out.println("movw %bx,%es");
				}
			}
			else {
				System.out.println("_" + pref + "start:");
				System.out.println("movw %cs,%bx");
				System.out.println("addw $" + pref + "RESrmstroff,%bx");
				System.out.println("movw %bx,%es");
			}
			while ((tin = inr.readLine()) != null) {
				line++;
				tin = tin.trim();
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
					if ((n > 15) || (n < 0)) {
						throw new Exception ("Argument count (" + Integer.toString(n) + ") is not in the range [0, 16]");
					}
					System.out.println(pref + "func_" + tre[1] + "_" + Integer.toString(n) + ":");
					fuo = true;
					continue;
				}
				if (tre[0].equals("lbl")) {
					System.out.println(pref + "label_" + tre[1] + ":");
					continue;
				}
				if (tre[0].equals("if")) {
					solv(Arrays.copyOfRange(tre, 1, tre.length));
					System.out.println("cmpw $0x0000,%ax");
					blocks.push(blex);
					brer++;
					blex = brer;
					System.out.println("jz " + pref + "block_" + Integer.toString(blex));
					blid.push(blo);
					blo = 1;
				}
				if (tre[0].equals("endif")) {
					if (blo != 1) {
						throw new Exception("Use of \"endif\" without being in an innermost conditional block of type \"if\"");
					}
					System.out.println(pref + "block_" + Integer.toString(blex) + ":");
					try {
						blex = blocks.pop();
						blo = blid.pop();
					}
					catch (EmptyStackException E) {
						throw new Exception("Use of \"endif\" outside of a conditional block");
					}
				}
				if (tre[0].equals("while")) {
					blocks.push(blex);
					brer++;
					blex = brer;
					System.out.println(pref + "wstart_" + Integer.toString(blex) + ":");
					solv(Arrays.copyOfRange(tre, 1, tre.length));
					System.out.println("cmpw $0x0000,%ax");
					System.out.println("jz " + pref + "block_" + Integer.toString(blex));
					blid.push(blo);
					blo = 2;
				}
				if (tre[0].equals("wend")) {
					if (blo != 2) {
						throw new Exception("Use of \"wend\" without being in an innermost conditional block of type \"while\"");
					}
					System.out.println("jmp " + pref + "wstart_" + Integer.toString(blex));
					System.out.println(pref + "block_" + Integer.toString(blex) + ":");
					try {
						blex = blocks.pop();
						blo = blid.pop();
					}
					catch (EmptyStackException E) {
						throw new Exception("Use of \"wend\" outside of a conditional block");
					}
				}
				if (tre[0].equals("goto")) {
					System.out.println("jmp " + pref + "label_" + tre[1]);
					continue;
				}
				if (tre[0].equals("return")) {
					solv(Arrays.copyOfRange(tre, 1, tre.length));
					System.out.println("movw %ax,%ds:0x1e");
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
						for (reer = 0; reer < c; reer++) {
							System.out.println("pushw %ds:" + cleanse(reer * 2));
						}
						System.out.println("call " + pref + "func_" + tre[1] + "_" + Integer.toString(c));
						reer--;
						for (;reer >= 0; reer--) {
							System.out.println("popw %ds:" + cleanse(reer * 2));
						}
						continue;
					}
					System.out.println("call " + pref + "func_" + tre[1] + "_0");
					continue;
				}
				if (tre[0].equals("decl")) {
					vars.add((new Var()).name(tre[1]).pos(n = (Var.curp = Var.curp + 2)));// TODO Optimize variable memory usage
					vans.add(tre[1]);
					solv(Arrays.copyOfRange(tre, 2, tre.length));
					System.out.println("movw %ax,%ds:" + cleanse(n));
					continue;
				}
				if (tre[0].equals("declstr")) {
					vass.add((new Svar()).name(tre[1]).cont(tin.substring(tin.indexOf('\"') + 1, tin.lastIndexOf('\"'))));
					vees.add(tre[1]);
					continue;
				}
//				if ((!vans.contains(tre[0])) && (!tre[0].startsWith("arg"))) {
//					throw new Exception("Undefined variable \"" + tre[0] + "\"");
//				}
				if (tre.length == 1) {
					if (tre[0].charAt(0) == '!') {
						tre[0] = tre[0].substring(1);
						System.out.println("movw $0x0,%ds:" + cleanse(locv(tre[0])));
						continue;
					}
					System.out.println("movw $0x1,%ds:" + cleanse(locv(tre[0])));
					continue;
				}
				if (!tre[1].equals("=")) {
					throw new Exception("Illegal operator: Expected '='");
				}
				sols(tre);
			}
			if ((!(blocks.empty())) || (!(blid.empty()))) {
				throw new Exception("Unclosed coditional block(s)");
			}
			System.out.println(".space (16 - ((. - " + pref + "_start) % 16)) % 16");
			System.out.println(".set " + pref + "RESrmstroff, (. - " + pref + "_start) / 16");
			System.out.println(pref + "RESstrstart:");
			for(Svar vs : vass) {
				System.out.println(pref + "str_" + vs.name + ':');
				System.out.println(".asciz \"" + vs.cont + "\"");
				System.out.println(".set " + pref + "str_" + vs.name + "_len, ( . - str_" + vs.name + " - 1 )");
				System.out.println(".set " + pref + "str_" + vs.name + "_addr, ( str_" + vs.name + " - " + pref + "RESstrstart )");
			}
		}
		catch (Exception E) {
			System.out.println("Line " + line + ": " + E);
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
//		int dep = 0;
//		int mdep = 0;
//		for (String s : ss) {
//			if (s.equals("(")) {
//				dep++;
//				mdep = ((dep > mdep) ? dep : mdep);
//			}
//			else if (s.equals(")")) {
//				dep--;
//			}
//		}
//		if (dep != 0) {
//			throw new Exception("Unbalanced parentheses");
//		}
//		if (mdep == 0) {
		int j;
		int dept;
		int eg;
		int dah = 0;
		boolean brc = false;
		if (!ss[0].equals("(")) {
			bring(ss[0], "ax");
			j = 0;
		}
		else {
			j = dept = 1;
			if (ss[1].equals(")")) {
				throw new Exception("Empty parentheses");
			}
			try {
				while (dept != 0) {
					if (ss[j].equals("(")) {
						dept++;
					}
					else if (ss[j].equals(")")) {
						dept--;
					}
					j++;
				}
			}
			catch (ArrayIndexOutOfBoundsException E) {
				throw new Exception("Parentheses not closed");
			}
			j--;
			solv(Arrays.copyOfRange(ss, 1, j));
		}
		for (; j < (ss.length - 1); j += 2) { // TODO Implement order of operation
			if (ss[j + 2].equals("(")) {
				System.out.println("pushw %ax");
				eg = j;
				j += 3;
				if (ss[j + (dept = 1)].equals(")")) {
					throw new Exception("Empty parentheses");
				}
				try {
					while (dept != 0) {
						if (ss[j].equals("(")) {
							dept++;
						}
						else if (ss[j].equals(")")) {
							dept--;
						}
						j++;
					}
				}
				catch (ArrayIndexOutOfBoundsException E) {
					throw new Exception("Parentheses not closed");
				}
				j--;
				solv(Arrays.copyOfRange(ss, eg + 3, j));
				dah = j;
				j = eg;
				brc = true;
				System.out.println("movw %ax,%bx");
				System.out.println("popw %ax");
			}
			else {
				bring(ss[j + 2], "bx");
			}
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
					System.out.println("xorw %dx,%dx");
					System.out.println("divw %bx");
					break;
				case ('%'):
					System.out.println("xorw %dx,%dx");
					System.out.println("divw %bx");
					System.out.println("movw %dx,%ax");
					break;
				case ('*'):
					System.out.println("mulw %bx");
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
			if (brc) {
				j = dah - 2;
				brc = false;
			}
		}
//		}
		return;
	}
	static void bring(String vel, String reg) throws Exception {
		try {
			System.out.println("movw $" + cleanse(vel) + ",%" + reg);
		}
		catch (NumberFormatException E) {
			if (vel.equals("res")) {
				System.out.println("movw %ds:0x1e,%" + reg);
				return;
			}
			try {
				System.out.println("movw %ds:" + cleanse(locv(vel)) + ",%" + reg);
			}
			catch (VariableUndefinedException F) {
				String[] ves = vel.split("_");
				if (vees.contains(ves[0])) {
					if (ves.length == 1) {
						throw new Exception("String field not present");
					}
					if (ves[1].equals("len")) {
						System.out.println("movw $" + pref + "str_" + ves[0] + "_len,%" + reg);
						return;
					}
					if (ves[1].equals("addr")) {
						System.out.println("movw $" + pref + "str_" + ves[0] + "_addr,%" + reg);
						return;
					}
					throw new Exception("Unknown string field: \"" + ves[1] + "\"");
				}
				else {
					throw new VariableUndefinedException(F.getMessage() + " (also does not resolve to a string constant");
				}
			}
		}
	}
	static int locv(String n) throws Exception {
		if (n.startsWith("arg")) {
			int t = Integer.decode(n.substring(3));
			if ((t < 0) || (t > 15)) {
				throw new Exception("Illegal argument index");
			}
			return t * 2;
		}
		if (n.equals("res")) {
			return 30;
		}
		int i = vans.indexOf(n);
		if (i == (-1)) {
			throw new VariableUndefinedException("Undefined variable \"" + n + "\"");
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
	public boolean equals(Object n) {
		return name.equals(((Svar) n).name);
	}
}
@SuppressWarnings("serial")
class VariableUndefinedException extends Exception {
	VariableUndefinedException(String reas) {
		super(reas);
	}
}
