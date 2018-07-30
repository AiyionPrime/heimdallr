# Heimdallr

finding public ssh keys with ease


This tool is intended to get developers public keys
* via a limited ssh-server on the local host (which can be accessed by ssh-copy-id)
* or by fetching them from github, were the developers may've left them.

## Getting Started

These instructions will get you a copy of the project running on your local machine, for development and testing purposes. See [deployment](#deployment) for notes on how to deploy the project on a live system.

### Prerequisites


* `gcc` or another compiler
* [json-c](https://github.com/json-c/json-c) in order to interprete api responses
* [openssl](https://github.com/openssl/openssl) for generating ssh keys
* [libssh](https://www.libssh.org/) to run the ssh-server


### Installing

A step by step series of examples that tells you, how to get a development env running.

clone this repo

```
git clone git@github.com:aiyionprime/heimdallr.git
```

cd into the project and build it

```
cd heimdallr
make
```

Take a look at eg. my pubkey(s), by calling heimdallr with the user flag

```
./heimdallr -u aiyionprime
```

Alternatively start a ssh-server (eg. on port 1234), which others can fill with pubkeys via `ssh-copy-id`:

```
./heimdallr -p 1234
```

Your coworkers can then use `ssh-copy-id` like they would do with ordinary ssh-servers:

```
ssh-copy-id -p 1234 <the-ip-of-your-device>
```

Hint: You can list your current IPs with

```
hostname -i
```


## Deployment

#### From source

Deploying this on a live system can be done by running the [build steps](#installing) followed by

```
sudo make install
```

#### Using the arch user repository

```
yaourt -S heimdallr-git
```

If you like this tool, don't forget to vote for it, so others can find it, too!

https://aur.archlinux.org/pkgbase/heimdallr-git/ (the button is hidden on the right under 'package actions')
Thanks!


## Contributing

We're open for ideas improving the workflow of sharing ssh public keys,
so if you've got a good one, feel free to contact me about it.

## Author

aiyion (on #hackint)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Acknowledgments

* Thanks to all testers and reviewers!
* Props to ubuntus developers for using the same mechanism in their installer for bionic beaver.
