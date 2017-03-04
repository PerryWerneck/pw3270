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
 * programa; se não, escreva para a Free Software Foundation, Inc., 51 Franklin
 * St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Este programa está nomeado como private.h e possui - linhas de código.
 *
 * Contatos:
 *
 * perry.werneck@gmail.com	(Alexandre Perry de Souza Werneck)
 * erico.mendonca@gmail.com	(Erico Mascarenhas Mendonça)
 *
 */

#ifndef PRIVATE_H_INCLUDED

	#define PRIVATE_H_INCLUDED

	#include <config.h>
	#include <cstring>
	#include <pw3270cpp.h>

	#if defined(_WIN32)

		class recursive_mutex {
		private:
			HANDLE hMutex;

		public:
			recursive_mutex() {
				hMutex = CreateMutex(NULL,FALSE,NULL);
			};

			~recursive_mutex() {
				CloseHandle(hMutex);
			};

			void lock(void) {
				WaitForSingleObject(hMutex,INFINITE);
			};

			void unlock(void) {
				ReleaseMutex(hMutex);
			};

			bool try_lock(void) {
				if(WaitForSingleObject(hMutex,1) == WAIT_OBJECT_0)
					return true;
				return false;
			};
		};

	#elif __cplusplus < 201103L

		#define nullptr	NULL

		class recursive_mutex {
		private:
			pthread_mutex_t           mtx;
			pthread_mutexattr_t       mtxAttr;

		public:
			recursive_mutex() {

				memset(&mtx,0,sizeof(mtx));
				memset(&mtxAttr,0,sizeof(mtxAttr));

				pthread_mutexattr_init(&mtxAttr);
				pthread_mutexattr_settype(&mtxAttr, PTHREAD_MUTEX_RECURSIVE);
				pthread_mutex_init(&mtx, &mtxAttr);
			};

			~recursive_mutex() {
				pthread_mutex_destroy(&mtx);
			};

			void lock(void) {
				pthread_mutex_lock(&mtx);
			};

			void unlock(void) {
				pthread_mutex_unlock(&mtx);
			};

			bool try_lock(void) {
				 return pthread_mutex_trylock(&mtx) == 0;
			};
		};

	#else

		#include <mutex>

	#endif // !c11

	 namespace PW3270_NAMESPACE
	 {

		session	* create_service_client(const char *session) throw (std::exception);

	 }


#endif // PRIVATE_H_INCLUDED
