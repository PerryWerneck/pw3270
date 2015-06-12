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
	/* Get library/extension info */
	public native String	getVersion();
	public native String	getRevision();

	/* Connect/Disconnect */
	public native int		Connect( String uri, int timeout );
	public native int		Disconnect();
	public native int		getConnectionState();

	/* Get Status */
	public native boolean	isConnected();
	public native boolean	isTerminalReady();
	public native String	getEncoding();

	/* Get/Query Screen contents */
	public native String	getScreenContentAt(int row, int col, int size);
	public native String	getScreenContent();
	public native boolean	queryStringAt(int row, int col, String key);

	/* Actions/Screen changes */
	public native int		sendEnterKey();
	public native int		setStringAt(int row, int col, String str);
	public native int		sendPFKey(int key);

	/* Waiting */
	public native int		wait(int seconds);
	public native int		waitForTerminalReady(int timeout);
	public native int		waitForStringAt(int row, int col, String key, int timeout);

	/* Non-native methods */
	public int Connect(String hostinfo)
	{
		int rc = Connect(hostinfo,10);
		if(rc != 0)
			return rc;
		return waitForTerminalReady(10);
	}

	static
	{
		System.loadLibrary("jni3270");
	}

};
