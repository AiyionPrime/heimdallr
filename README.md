# Heimdallr

finding public ssh keys with ease

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

What things you need to install the software and how to install them


* `gcc` or another compiler
* [json-c](https://github.com/json-c/json-c) in order to interprete api responses


### Installing

A step by step series of examples that tell you how to get a development env running

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

## Deployment

#### From source

Deploying this on a live system can be done by running the build steps followed by

```
sudo make install
```

#### Using the arch user repository

```
yaourt -S heimdallr-git
```

If you like this tool, don't forget to vote for it, so others can find it, too!

https://aur.archlinux.org/pkgbase/heimfdallr-git/ (the button is hidden on the right under 'package actions')
Thanks!


## Contributing

We're open for ideas improving the workflow of sharing ssh public keys,
so if you've got a good one, feel free to contact me about it.

## Author

aiyion (on #hackint)

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

## Acknowledgments

* Thanks to all testers!
* Props to ubuntus developers for using the same mechanism in their installer for bionic beaver.
