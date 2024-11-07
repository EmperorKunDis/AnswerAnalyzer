#ifndef ANSWER_TRACKER_H
#define ANSWER_TRACKER_H

#include <string>
#include <vector>
#include <utility>
#include <stdexcept>
#include <tuple>

// Custom exception class for AnswerTracker-specific errors
class AnswerTrackerException : public std::runtime_error {
public:
    explicit AnswerTrackerException(const std::string& message) 
        : std::runtime_error(message) {}
};

// Structure to hold a pair of expected and actual answers
struct AnswerPair {
    std::string expected;
    std::string actual;
    
    AnswerPair(const std::string& exp, const std::string& act) 
        : expected(exp), actual(act) {}
};

class AnswerTracker {
private:
    std::vector<AnswerPair> answerPairs;
    double successPercentage;
    const size_t maxAnswers;
    
    // Helper functions
    std::string toLowerCase(const std::string& str) const;
    void validateInput(const std::string& input) const;

public:
    // Constructor
    AnswerTracker();
    
    // Core functionality
    bool addAnswer(const std::string& expected, const std::string& actual);
    void setSuccessPercentage();
    std::vector<std::tuple<int, bool, std::string, std::string>> analyzeResults() const;
    void displayResults() const;
    
    // File operations
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    
    // Interactive functionality
    void interactiveInput();
    
    // Getters
    double getSuccessPercentage() const { return successPercentage; }
    size_t getTotalAnswers() const { return answerPairs.size(); }
    size_t getMaxAnswers() const { return maxAnswers; }
    
    // Utility functions
    void clear() { answerPairs.clear(); successPercentage = 0.0; }
};

#endif // ANSWER_TRACKER_H