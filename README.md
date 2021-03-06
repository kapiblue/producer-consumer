# Producer-Consumer 

## Table of contents
* [Introduction](#introduction)
* [Setup](#setup)
* [Features](#features)
* [Sources](#sources)

## Introduction
The project implements the producer and the consumer to solve the Producer-Consumer problem. It was created as a final project for the Operating Systems and Concurrent Programming course. 

## Setup
Compile with g++. Running multiple producers and consumers is allowed.

## Features

* Supports running several producers and consumers at the same time
* Uses a circular buffer of size N to support up to N concurrent processes (by default N=5)
* The ordering of items being consumed reflects production end times

## Sources
The project was developed using the materials provided by PHD Cezary Sobaniec, PUT. 
