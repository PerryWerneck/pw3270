/*
 * "Software pw3270, desenvolvido com base nos códigos fontes do WC3270  e X3270
 * (Paul Mattes Paul.Mattes@usa.net), de emulação de terminal 3270 para acesso a
 * aplicativos mainframe. Registro no INPI sob o nome G3270.
 *
 * Copyright (C) <2008> <Banco do Brasil S.A.>
 *
 * Este programa é software livre. Você pode redistribuí-lo e/ou modificá-lo sob
 * os termos da GPL v.2 - Licença Pública Geral  GNU,  conforme  publicado  pela
 * Free Software Foundation.
 *
 * Este programa é distribuído na expectativa de  ser  útil,  mas  SEM  QUALQUER
 * GARANTIA; sem mesmo a garantia implícita de COMERCIALIZAÇÃO ou  de  ADEQUAÇÃO
 * A QUALQUER PROPÓSITO EM PARTICULAR. Consulte a Licença Pública Geral GNU para
 * obter mais detalhes.
 *
 * Você deve ter recebido uma cópia da Licença Pública Geral GNU junto com este
 * programa;  se  não, escreva para a Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA, 02111-1307, USA
 *
 * Este programa está nomeado como lib3270.java e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

package pw3270;

public class terminal
{
	// lib3270's session handle
	private long nativeHandle;

	// Init/Deinit
	private native int init();
	private native int init(String id);
	private native int deinit();

	// Get library/extension info
	public native String	get_version();
	public native String	get_revision();
	public native String	get_encoding();

	// Get/Set/Text with charset translation
	public native String	get_string(int baddr, int len);
	public native String	get_string_at(int row, int col, int sz);
	public native int		set_string_at(int row, int col, String str);
	public native int		cmp_string_at(int row, int col, String text);
	public native int		input_string(String str);

	// Cursor management
	public native int		set_cursor_position(int row, int col);
	public native int		set_cursor_addr(int addr);
	public native int		get_cursor_addr();

	// Keyboard actions
	public native int		enter();
	public native int		pfkey(int key);
	public native int		pakey(int key);

	// Connect/Disconnect
	public native int		connect(String host, int seconds);
	public native int		disconnect();

	public terminal() {
		init();
	}

	public terminal(String id) {
		init(id);
	}

	protected void finalize( ) throws Throwable {
		deinit();
	}

	static
	{
		System.loadLibrary("jni3270");
	}

};
