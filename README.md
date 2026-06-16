# ⚙️ Multithreaded File Processing CLI Tool

High-performance C++ CLI application for processing large datasets (CSV / JSON) with parallel execution and modular pipeline architecture.

---

## 🚀 Features

* ⚡ Multithreaded processing using custom ThreadPool
* 📂 Support for CSV and JSON file formats
* 🧩 Modular processing pipeline (extensible stages)
* 📊 Report generation system
* 📈 Progress tracking
* 🖥️ CLI interface with argument parsing
* 🧵 Safe concurrency using mutex and condition variables

---

## 🏗️ Architecture

The project is designed with a modular and scalable architecture:

* **ThreadPool** — manages worker threads and task scheduling
* **Processing Pipeline** — stages for transforming data
* **Parser Layer** — handles CSV / JSON input
* **Core Engine** — coordinates execution flow
* **CLI Layer** — handles user input and configuration
* **Report Module** — generates structured output

---

## 🔧 Technologies

* C++ (C++11+)
* STL (threading, containers, algorithms)
* Multithreading:

  * `std::thread`
  * `std::mutex`
  * `std::condition_variable`
* JSON: nlohmann/json
* File I/O

---

## ▶️ Usage

### Build

```bash
g++ -std=c++11 -pthread main.cpp -o processor
```

### Run

```bash
./processor --input data.csv --output report.txt
```

---

## 📊 Example Workflow

1. Load input file (CSV / JSON)
2. Split data into tasks
3. Process tasks in parallel using ThreadPool
4. Aggregate results
5. Generate report

---

## 🧠 Key Concepts Implemented

* Custom task scheduling
* Priority-based execution
* Pipeline-style data processing
* Exception handling in async tasks
* Resource management and cleanup
* Basic fault tolerance

---

## 📌 Future Improvements

* Add unit tests
* Improve error handling and logging
* Support more file formats
* Add configuration file support
* Optimize memory usage
* Benchmarking and performance metrics

---

## 👨‍💻 Author

Anatolii K.
Junior C++ Developer (Systems / Backend)

GitHub:
https://github.com/Anatolijhack

---

## 💡 Notes

This project was built as a learning and portfolio project to explore:

* multithreading
* systems programming
* data processing pipelines

The focus is on clean architecture, performance, and extensibility.
