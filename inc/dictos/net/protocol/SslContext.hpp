#pragma once

namespace dictos::net::protocol {

/**
 * The ssl context class helps setup the boost asio ssl context.
 */
class SslContext : public config::Context
{
public:
	SslContext(config::Options options, bool server = false) :
		Context(getSection(), std::move(options)), m_server(server),
		m_context(std::make_unique<boost::asio::ssl::context>(boost::asio::ssl::context::tls_client))
	{
		setup();
	}

	SslContext(SslContext &&context) :
		m_context(std::move(context.m_context)),
		Context(std::move(context))
	{
	}

	SslContext & operator = (SslContext &&context)
	{
		m_context = std::move(context.m_context);
		return *this;
	}

	SSL_CTX * nativeContext() { return m_context->native_handle(); }

	operator boost::asio::ssl::context &() { return *m_context; }
	operator const boost::asio::ssl::context &() const { return *m_context; }

protected:
	void setup()
	{
		auto client_cert_file = getOption<file::path>("client_cert_file");
		auto private_key_file = getOption<file::path>("private_key_file");
		auto cert_chain_file = getOption<file::path>("cert_chain_file");
		auto verify_peer = getOption<bool>("verify_peer");
		auto cipher_list = getOption<std::string>("cipher_list");

		LOGT(CRITICAL, "Client cert file:", client_cert_file);
		LOGT(CRITICAL, "Private key file:", private_key_file);
		LOGT(CRITICAL, "Cert chain file::", cert_chain_file);
		LOGT(CRITICAL, "Verify peer:", verify_peer);
		LOGT(CRITICAL, "Cipher list:", cipher_list);

		m_context->set_options(
			boost::asio::ssl::context::no_sslv2		|
			boost::asio::ssl::context::no_sslv3		|
			boost::asio::ssl::context::no_tlsv1		|
			boost::asio::ssl::context::no_tlsv1_1	|
			boost::asio::ssl::context::no_compression
		);

		m_context->use_certificate_chain_file(cert_chain_file.string());
		m_context->load_verify_file(cert_chain_file.string());

		m_context->use_certificate_file(client_cert_file.string(), boost::asio::ssl::context::file_format::pem);
		m_context->use_private_key_file(private_key_file.string(), boost::asio::ssl::context::file_format::pem);

		if (verify_peer) {
			if (m_server)
				m_context->set_verify_mode(boost::asio::ssl::verify_peer|boost::asio::ssl::verify_fail_if_no_peer_cert);
			else
				m_context->set_verify_mode(boost::asio::ssl::verify_peer);
		}

		SSL_CTX_set_cipher_list(m_context->native_handle(), cipher_list.c_str());
	}

	static const config::Section & getSection()
	{
		if (auto section = config::Sections::find("net_ssl"))
			return *section;

		static config::Section section("net_ssl", {
				{"private_key_file", file::path(), "Path to client private ke (for client based auth)y"},
				{"client_cert_file", file::path(), "Path to client cert file key (for client based auth)"},
				{"cert_chain_file", file::path(), "Path to cert chain file (for peer certificate validation)"},
				{"verify_peer", true, "Whether to verify the peer" },
				{"cipher_list", "HIGH:!DSS:!aNULL@STRENGTH"s, "The ssl cipher list to control cipher selection"},
			}
		);

		return section;
	}

	std::unique_ptr<boost::asio::ssl::context> m_context;
	bool m_server;	// Indicates whether we'll be used as a client or a server so we can set options appropriately 
};

}
