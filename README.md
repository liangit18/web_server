# web_server

INTRODUCTION:
This is a simple Web server developed in C, it can response HTTP/1.0 with two common methods(GET and HEAD). In default way, this web server can return index.html or list of files in default directory. Besides, this server also provides CGI to user, which allows user dynamically access server. This web server accept both IPv4 and Ipv6, and allows user to choose that listening to certain IP address, port or any IP address. There are two running modes: debugging mode and daemon process mode. When running as a daemon process, it can response requests in parallel way(multiprocessing or multithreadin).

SYNOPSIS:
	sws [-dht] [-c cgi_dir] [-i ip_address] [-p port] [-l file] dir

DESCRIPTION:
	dir: this is the work space of web server, all the other operations will base on this directory.

	-d: running in debugging mode. Running in this mode, server processes only one request every time. Running log will be printed immediately.
	-h: print manual.
	-t: running in multi-thread mode(By default, daemon process mode running in multi-process way).
	-c cgi_dir: if server is executed without this option, server will treat any url as regular directory.If running with this option, url contains 'cgi-bin'(like address/cgi-bin/example.cgi?q=foo&food=bacon) will be treat as cgi request.And server will execute dir/cgi_dir/example.cgi.
	-i ip_address: server will listen 'ip_address' only.
	-p port: server will listen 'port' only
	-l file: stores running log information in /dir/file
