/*
 * The MIT License (MIT)

 * Copyright (c) 2023 Gary Sweet
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include "Socket.h"

#include <iostream>

#include "sockpp/tcp_acceptor.h"
#include "sockpp/version.h"

using namespace std;

constexpr uint32_t PORT = 6666;

static void DataThread(Socket *socket)
{
	in_port_t port = PORT;

	sockpp::initialize();
	sockpp::tcp_acceptor acc(port);

	if (!acc)
		throw "Error creating socket acceptor";

	while (true)
	{
		sockpp::inet_address peer;

		// Accept a new client connection
		sockpp::tcp_socket sock = acc.accept(&peer);

		if (!sock)
		{
			cerr << "Error accepting incoming connection: "
				 << acc.last_error_str() << endl;
		}
		else
		{
		    sock.set_option(SOL_SOCKET, SO_REUSEADDR, 1);

            ssize_t n;
            char    buf[4096];

            while ((n = sock.read(buf, sizeof(buf))) > 0)
            {
                buf[n - 1] = '\0';
				socket->SetData(buf);
            }
		}
	}
}

Socket::Socket()
{
	m_thread = std::thread(DataThread, this);
}

// NOTE: no destructor or thread cleanup, this code always just runs forever

void Socket::SetData(const string &data)
{
	{
		std::lock_guard lock(m_mutex);
		m_data = data;
	}
	m_cv.notify_one();
}

std::string Socket::GetDataBlocking()
{
	std::string ret;

	std::unique_lock lock(m_mutex);
	m_cv.wait(lock, [this]{ return !this->m_data.empty(); });

	std::swap(ret, m_data);
	return ret;
}
