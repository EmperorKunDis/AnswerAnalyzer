#ifndef ANSWER_ANALYZER_H
#define ANSWER_ANALYZER_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <optional>
#include <stdexcept>

// Custom exception class for AnswerAnalyzer-specific errors
class AnswerAnalyzerException : public std::runtime_error {
public:
    explicit AnswerAnalyzerException(const std::string& message) 
        : std::runtime_error(message) {}
};

struct TestAttempt {
    std::vector<std::string> answers;
    double percentage;
    
    TestAttempt(const std::vector<std::string>& ans, double perc) 
        : answers(ans), percentage(perc) {}
};

class AnswerAnalyzer {
private:
    std::vector<TestAttempt> attempts;
    size_t maxAnswers;
    std::vector<std::vector<bool>> possibleCombinations;
    bool combinationsCalculated;
    std::vector<std::optional<bool>> definiteAnswers;  // true = correct, false = incorrect, nullopt = unknown
    
    void updatePossibleCombinations();
    bool isValidCombination(const std::vector<bool>& combination) const;
    void updateDefiniteAnswers();
    
public:
    // Constructor
    AnswerAnalyzer(size_t maxAns = 10) 
        : maxAnswers(maxAns), combinationsCalculated(false) {
        definiteAnswers.resize(maxAnswers);
    }
    
    // Core functionality
    void addAttempt(const std::vector<std::string>& answers, double percentage);
    void analyzeResults() const;
    void clear();
    
    // Analysis methods
    std::vector<std::string> getMostCommonAnswers() const;
    std::vector<std::pair<std::string, double>> getAnswerConfidences() const;
    std::map<size_t, std::vector<std::string>> getAnswerPatterns() const;
    std::vector<std::string> suggestNextAttempt() const;
    double predictScore(const std::vector<std::string>& answers) const;
    
    // Statistics
    double getAverageScore() const;
    double getScoreVariance() const;
    
    // File operations
    void saveToFile(const std::string& filename) const;
    void loadFromFile(const std::string& filename);
    
    // Getters
    const std::vector<std::optional<bool>>& getDefiniteAnswers() const { return definiteAnswers; }
    size_t getNumAttempts() const { return attempts.size(); }
    size_t getMaxAnswers() const { return maxAnswers; }
    size_t getFirstAttemptSize() const { 
        return attempts.empty() ? maxAnswers : attempts[0].answers.size(); 
    }
};

#endif