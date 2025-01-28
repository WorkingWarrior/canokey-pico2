# CanoKey on Raspberry Pi Pico 2

<p align="center">
	<img src="https://img.shields.io/github/license/WorkingWarrior/canokey-pico2?style=default&logo=opensourceinitiative&logoColor=white&color=0080ff" alt="license">
	<img src="https://img.shields.io/github/last-commit/WorkingWarrior/canokey-pico2?style=default&logo=git&logoColor=white&color=0080ff" alt="last-commit">
	<img src="https://img.shields.io/github/languages/top/WorkingWarrior/canokey-pico2?style=default&color=0080ff" alt="repo-top-language">
	<img src="https://img.shields.io/github/languages/count/WorkingWarrior/canokey-pico2?style=default&color=0080ff" alt="repo-language-count">
</p>

This is an implementation of CanoKey for Raspberry Pi Pico 2.

**Warning:** \
This CanoKey port for Raspberry Pi Pico 2 is in early development stages and under active development. The code has not been thoroughly tested and may function incorrectly or fail to function. Use at your own discretion.

##  Overview

CanoKey is an open-source USB security token implementing multiple authentication standards and protocols. This port enables running CanoKey firmware on the Raspberry Pi Pico 2 microcontroller board.

### Key features:

* OpenPGP Card V3.4 with support for:
  - RSA
  - ECDSA
  - ED25519
* PIV Card
* Time-based and HMAC-based One-Time Passwords:
  - TOTP (RFC6238)
  - HOTP (RFC4226)
* U2F authentication
* FIDO2 (WebAuthn)

The software is compatible with modern operating systems including Linux, Windows, and macOS without requiring additional drivers. Please note that NFC functionality is not currently supported.

**Important security notice:**

This Raspberry Pi Pico 2 version comes with significant security limitations:

1. No security guarantees or warranties are provided
2. Physical access to the device enables extraction of all data, including secret keys
3. The implementation is vulnerable to side-channel attacks
4. Usage is entirely at your own risk

For a security-hardened version, please visit [canokeys.org](https://canokeys.org)

##  Getting Started

###  Prerequisites

To build and use canokey-pico2, you'll need:

- C/C++ development environment (GCC 9.0+ recommended)
- CMake (version 3.12 or higher)
- Raspberry Pi Pico SDK (1.5.0+)
- Micro USB cable for flashing
- Optional: Picoprobe or other SWD debugger for development

### Installation

Detailed installation instructions coming soon. The process will involve:

1. Setting up the build environment
   - Installing required packages
   - Configuring Pico SDK
2. Compiling the firmware
   - Getting source code
   - Building with CMake
3. Flashing to Raspberry Pi Pico 2
   - Using bootloader mode
   - Using SWD debugger
4. Initial device configuration
   - Setting up security parameters
   - Testing basic functionality

###  Usage

TBD

###  Testing

TBD

##  Contributing

- [Join the Discussions](https://github.com/WorkingWarrior/canokey-pico2/discussions): Share your insights, provide feedback, or ask questions.
- [Report Issues](https://github.com/WorkingWarrior/canokey-pico2/issues): Submit bugs found or log feature requests
- [Submit Pull Requests](https://github.com/WorkingWarrior/canokey-pico2/blob/main/CONTRIBUTING.md): Review open PRs, and submit your own PRs.

<details closed>
<summary>Contributing Guidelines</summary>

1. **Fork the Repository**: Start by forking the project repository to your github account.
2. **Clone Locally**: Clone the forked repository to your local machine using a git client.
   ```sh
   git clone https://github.com/WorkingWarrior/canokey-pico2
   ```
3. **Create a New Branch**: Always work on a new branch, giving it a descriptive name.
   ```sh
   git checkout -b new-feature-x
   ```
4. **Make Your Changes**: Develop and test your changes locally.
5. **Commit Your Changes**: Commit with a clear message describing your updates.
   ```sh
   git commit -m 'Implemented new feature x.'
   ```
6. **Push to github**: Push the changes to your forked repository.
   ```sh
   git push origin new-feature-x
   ```
7. **Submit a Pull Request**: Create a PR against the original project repository. Clearly describe the changes and their motivations.
8. **Review**: Once your PR is reviewed and approved, it will be merged into the main branch. Congratulations on your contribution!
</details>

<details closed>
<summary>Contributor Graph</summary>
<br>
<p align="left">
   <a href="https://github.com/WorkingWarrior/canokey-pico2/graphs/contributors">
      <img src="https://contrib.rocks/image?repo=WorkingWarrior/canokey-pico2">
   </a>
</p>
</details>

## License

This project is licensed under the [Apache 2.0 License](LICENSE).
