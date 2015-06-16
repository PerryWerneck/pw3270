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
	private native int		init();
	private native int		init(String id);
	private native int		deinit();

	/**
	 * Wait for terminal negociation.
	 * <p>
	 * Wait on a loop until the terminal contents are
	 * ready for reading.
	 *
	 * @param seconds Maximum time (in seconds) to wait for.
	 */
	public native int wait_for_ready(int seconds);

	/**
	 * Get the current lib3270 version.
	 *
	 * @return String with the current lib3270 version.
	 *
	 */
	public native String	get_version();

	/**
	 * Get the current lib3270 revision.
	 *
	 * @return String with the current lib3270 revision.
	 *
	 */
	public native String	get_revision();

	/**
	 * Get the current lib3270 encoding.
	 *
	 * @return String with the current lib3270 encoding (ISO-8859-1).
	 *
	 */
	public native String	get_encoding();

	// Get/Set/Text with charset translation

	/**
	 * Get terminal contents starting at desired address.
	 *
	 * @param baddr	Address of the beginning of the string.
	 * @param len	Length of the string.
	 *
	 * @return Terminal contents with "len" bytes starting at "baddr" position.
	 *
	 */
	public native String	get_string(int baddr, int len);

	/**
	 * Get terminal contents starting at row, col.
	 *
	 * @param row	Row of the text.
	 * @param col	Column of the text.
	 * @param sz	Size to read.
	 *
	 * @return Contents of terminal at row, col with sz bytes.
	 *
	 */
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

	// Actions
	public native int		quit();
	public native int		erase_eof();
	public native int		print();

	// Connect/Disconnect

	/**
	 * Connect to 3270 host.
	 * <p>
	 * Connect to the 3270 host
	 * <p>
	 * URI formats:
	 * <ul>
	 * <li>tn3270://[HOSTNAME]:[HOSTPORT] for non SSL connections.</li>
	 * <li>tn3270s://[HOSTNAME]:[HOSTPORT] for ssl connection.</li>
	 * </ul>
	 *
	 * @param host		Host URI.
	 * @param seconds	How many seconds to wait for a connection.
	 *
	 */
	public native int		connect(String host, int seconds);

	/**
	 * Disconnect from host.
	 *
	 */
	public native int		disconnect();

	/**
	 * Creates a tn3270 terminal without associating it
	 * with any pw3270 window.
	 *
	 */
	public terminal() {
		init();
	}

	/**
	 * Creates a tn3270 terminal associated with a
	 * pw3270 window.
	 *
	 * @param id String identifying the target window.
	 *
	 */
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
