#include "answerTracker.h"
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <fstream>

// Constructor
AnswerTracker::AnswerTracker() 
    : successPercentage(0.0), maxAnswers(10) {}

// Helper function to convert string to lowercase
std::string AnswerTracker::toLowerCase(const std::string& str) const {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

// Input validation helper
void AnswerTracker::validateInput(const std::string& input) const {
    if (input.empty()) {
        throw AnswerTrackerException("Input cannot be empty");
    }
    if (input.length() > 100) {
        throw AnswerTrackerException("Input is too long (max 100 characters)");
    }
}

// Core function to add an answer pair
bool AnswerTracker::addAnswer(const std::string& expected, const std::string& actual) {
    try {
        validateInput(expected);
        validateInput(actual);
        
        if (answerPairs.size() >= maxAnswers) {
            return false;
        }
        
        answerPairs.emplace_back(expected, actual);
        return true;
    }
    catch (const AnswerTrackerException& e) {
        throw;
    }
}

// Calculate success percentage
void AnswerTracker::setSuccessPercentage() {
    if (answerPairs.empty()) {
        successPercentage = 0.0;
        return;
    }
    
    size_t correctCount = 0;
    for (const auto& pair : answerPairs) {
        if (toLowerCase(pair.expected) == toLowerCase(pair.actual)) {
            correctCount++;
        }
    }
    
    successPercentage = (static_cast<double>(correctCount) / answerPairs.size()) * 100.0;
}

// Analyze and return results
std::vector<std::tuple<int, bool, std::string, std::string>> 
AnswerTracker::analyzeResults() const {
    std::vector<std::tuple<int, bool, std::string, std::string>> results;
    
    for (size_t i = 0; i < answerPairs.size(); ++i) {
        const auto& pair = answerPairs[i];
        bool isCorrect = toLowerCase(pair.expected) == toLowerCase(pair.actual);
        results.emplace_back(i + 1, isCorrect, pair.expected, pair.actual);
    }
    
    return results;
}

// Display results to console
void AnswerTracker::displayResults() const {
    std::cout << "\n=== Results Analysis ===" << std::endl;
    std::cout << "Total Questions: " << answerPairs.size() << std::endl;
    std::cout << "Success Rate: " << std::fixed << std::setprecision(1) 
              << successPercentage << "%" << std::endl << std::endl;
    
    for (const auto& [num, correct, expected, actual] : analyzeResults()) {
        std::cout << "Question " << num << ": " << (correct ? "✓" : "✗") << std::endl;
        std::cout << "  Expected: " << expected << std::endl;
        std::cout << "  Actual: " << actual << std::endl << std::endl;
    }
}

// Save results to file
void AnswerTracker::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file) {
        throw AnswerTrackerException("Cannot open file for writing: " + filename);
    }
    
    try {
        // Save answer pairs
        file << answerPairs.size() << "\n";
        for (const auto& pair : answerPairs) {
            file << pair.expected << "\n" << pair.actual << "\n";
        }
        
        // Save success percentage
        file << successPercentage << "\n";
        
        if (!file) {
            throw AnswerTrackerException("Error writing to file: " + filename);
        }
    }
    catch (...) {
        file.close();
        throw;
    }
    
    file.close();
}

// Load results from file
void AnswerTracker::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw AnswerTrackerException("Cannot open file for reading: " + filename);
    }
    
    try {
        clear();
        
        // Read number of pairs
        size_t numPairs;
        file >> numPairs;
        file.ignore(); // Skip newline
        
        // Read pairs
        for (size_t i = 0; i < numPairs; ++i) {
            std::string expected, actual;
            std::getline(file, expected);
            std::getline(file, actual);
            
            if (!addAnswer(expected, actual)) {
                throw AnswerTrackerException("Maximum answers limit reached while loading file");
            }
        }
        
        // Read success percentage
        file >> successPercentage;
        
        if (file.fail() && !file.eof()) {
            throw AnswerTrackerException("Error reading from file: " + filename);
        }
    }
    catch (...) {
        file.close();
        throw;
    }
    
    file.close();
}

// Interactive input function
void AnswerTracker::interactiveInput() {
    std::string expected, actual;
    std::cout << "\n=== Interactive Answer Input ===" << std::endl;
    
    while (getTotalAnswers() < getMaxAnswers()) {
        std::cout << "\nQuestion " << (getTotalAnswers() + 1) 
                 << " of " << getMaxAnswers() << std::endl;
        
        try {
            std::cout << "Enter expected answer (or 'quit' to finish): ";
            std::getline(std::cin, expected);
            
            if (toLowerCase(expected) == "quit") {
                break;
            }
            
            std::cout << "Enter actual answer: ";
            std::getline(std::cin, actual);
            
            if (!addAnswer(expected, actual)) {
                std::cout << "Failed to add answer pair!" << std::endl;
                break;
            }
        }
        catch (const AnswerTrackerException& e) {
            std::cout << "Error: " << e.what() << std::endl;
            std::cout << "Please try again." << std::endl;
            continue;
        }
    }
    
    setSuccessPercentage();
}