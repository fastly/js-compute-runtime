# compute-file-server

Fastly File Server uploads files to Fastly for serving directly from within Fastly Compute applications.

Upload any type of file: images, text, video etc and serve directly from Fastly.

It is ideal for serving files built from a static site generator such as 11ty.

## Install

### npm

Install pre-compiled binaries via `npm`

```sh
npm install compute-file-server
```

### Cargo

Compile and install via `cargo`

```sh
git clone https://github.com/JakeChampion/compute-file-server
cd compute-file-server/cli
cargo install --path .
```

## Commands

### Upload

Upload files to a Fastly Object Store, creating the Object Store if it does not exist.

Example: `compute-file-server upload --name website-static-files -- ./folder/of/files`

```sh
compute-file-server upload
Upload files

Usage: compute-file-server upload [OPTIONS] --name <NAME> -- <PATH>

Arguments:
  <PATH>  

Options:
      --name <NAME>    
      --token <TOKEN>  
  -h, --help           Print help information
```

### Link

Connect a Fastly Object Store to a Fastly Service.

Example: `compute-file-server link --name website-static-files --link-name files --service-id xxyyzz`

```sh
Usage: compute-file-server link [OPTIONS] --name <NAME> --link-name <LINK_NAME> --service-id <SERVICE_ID>

Options:
      --name <NAME>
      --token <TOKEN>
      --link-name <LINK_NAME>
      --service-id <SERVICE_ID>
  -h, --help                     Print help information
```

### Local

Update `fastly.toml` to contain a local Object Store containing the specified files.

Example: `compute-file-server local --name files --toml fastly.toml -- ./folder/of/files`

```sh
Usage: compute-file-server local --toml <TOML> --name <NAME> -- <PATH>

Arguments:
  <PATH>

Options:
      --toml <TOML>
      --name <NAME>
  -h, --help         Print help information
```