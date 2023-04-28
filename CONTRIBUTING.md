# Contributing to the JS Compute Runtime

First off thank you for wanting to contribute to making the JS Compute Runtime better! We
appreciate you taking time to improve the Compute@Edge experience for developers
everywhere. There are many ways you can contribute that include but aren't
limited to documentation, opening issues, issue triage, and code contributions.
We'll cover some of the ways you can contribute below, but if you don't see
instructions for what you want to do, open up an issue and ask us!

## Table of Contents
1. Documentation
1. Feature Requests
1. Bugs
1. Issue Triage
1. Code Contributions

## Documentation

Was something in our documentation unclear? Does something have no documentation,
but should? This is a perfect way to make an easy contribution to the JS Compute Runtime. If
you're not sure what the documentation should contain, please open up an issue!
We're happy to guide you with the correct information needed or if you already
know what needs to be done, open up a PR and we'll review it for you before
merging your changes.

## Feature Requests

Do you think there's something the JS Compute Runtime should have that would make the
experience better? Feature requests are a great way to let us know. Before
opening up a feature request on the issue tracker first make sure that there is
no currently open issue asking for the same thing. If there's not, open up an
issue asking for what you want and the motivation behind the change.

## Bugs

Sometimes you run into issues and the code is not working properly. If you do
run into a bug and you can't figure it out or if you do figure out the bug, open
up an issue on the issue tracker. Just make sure it's not already an issue that
has been filed yet. If you do open up an issue let us know what you expected to
happen, what actually happened, what your operating system is, as well as a case
we can use to reproduce the issue if you have one!

## Issue Triage

Sometimes issues get stale and are no longer an issue, need to be updated, or
have been fixed by a PR and were never closed. While we try to stay on top of
issues and keep the backlog groomed, we are only human and can miss out on
things. If you find that an issue can be closed,

## Code Contributions

If you want to contribute code to the JS Compute Runtime thank you! A few things before you do
get started adding a change and open up a PR

1. Make sure there's a tracking issue for your code change. We don't want you to
   do a lot of work only for us to reject the PR because it's a feature change
   we won't accept for instance.
1. Before opening your PR make sure things are working locally.
   You can test your resulting `.wasm` files locally using
   [Fastly's local testing server](https://developer.fastly.com/learning/compute/testing/#running-a-local-testing-server).

It also helps to understand how we structure the JS Compute Runtime code base.
- Under `src` is the code related to bulding a JS file and the JS runtime iself into a
`.wasm` file.
- Under `runtime/js-compute-runtime` is the code for the actual JS runtime itself.
  See below for building the runtime.
  - The main logic for initializing the JS runtime, reading the JS source code from `stdin`,
    and evaluating the JS top-level code is in `js-compute-runtime.cpp`.
  - The various builtins the JS Compute Runtime adds to JS are defined in
    `js-compute-builtins.cpp`.

Thanks again for contributing to the JS Compute Runtime. We really do appreciate you wanting to
help out and make it better!
