#include "answerAnalyzer.h"
#include <algorithm>
#include <numeric>
#include <fstream>
#include <cmath>
#include <map>
#include <set>

void AnswerAnalyzer::addAttempt(const std::vector<std::string>& answers, double percentage) {
    if (percentage < 0.0 || percentage > 100.0) {
        throw AnswerAnalyzerException("Percentage must be between 0 and 100");
    }
    
    if (answers.empty() || answers.size() > maxAnswers) {
        throw AnswerAnalyzerException("Invalid number of answers");
    }
    
    if (!attempts.empty() && answers.size() != attempts[0].answers.size()) {
        throw AnswerAnalyzerException("Number of answers must match previous attempts");
    }
    
    attempts.emplace_back(answers, percentage);
    combinationsCalculated = false;
}

void AnswerAnalyzer::analyzeResults() const {
    if (attempts.empty()) {
        throw AnswerAnalyzerException("No attempts to analyze");
    }
    
    // Basic analysis is performed by other methods when needed
    // This method could be expanded to perform more complex analysis
}

void AnswerAnalyzer::clear() {
    attempts.clear();
    possibleCombinations.clear();
    combinationsCalculated = false;
    definiteAnswers.clear();
    definiteAnswers.resize(maxAnswers);
}

std::vector<std::string> AnswerAnalyzer::getMostCommonAnswers() const {
    if (attempts.empty()) {
        return {};
    }
    
    std::vector<std::string> result;
    size_t numQuestions = attempts[0].answers.size();
    
    for (size_t q = 0; q < numQuestions; ++q) {
        std::map<std::string, int> answerCounts;
        for (const auto& attempt : attempts) {
            answerCounts[attempt.answers[q]]++;
        }
        
        auto maxElement = std::max_element(
            answerCounts.begin(), 
            answerCounts.end(),
            [](const auto& p1, const auto& p2) {
                return p1.second < p2.second;
            }
        );
        
        result.push_back(maxElement->first);
    }
    
    return result;
}

std::vector<std::pair<std::string, double>> AnswerAnalyzer::getAnswerConfidences() const {
    if (attempts.empty()) {
        return {};
    }
    
    std::vector<std::pair<std::string, double>> result;
    size_t numQuestions = attempts[0].answers.size();
    
    // Sort attempts by score to give more weight to higher-scoring attempts
    std::vector<const TestAttempt*> sortedAttempts;
    for (const auto& attempt : attempts) {
        sortedAttempts.push_back(&attempt);
    }
    std::sort(sortedAttempts.begin(), sortedAttempts.end(),
              [](const TestAttempt *a, const TestAttempt *b) {
                return a->percentage > b->percentage;
              });
    
    for (size_t q = 0; q < numQuestions; ++q) {
        std::map<std::string, std::vector<double>> answerScores;
        std::map<std::string, double> weightedScores;
        std::map<std::string, double> totalWeights;
        
        // First, collect all scores for each answer
        for (const auto& attempt : attempts) {
            answerScores[attempt.answers[q]].push_back(attempt.percentage);
        }
        
        // Calculate weighted scores with penalty for low scores
        for (size_t i = 0; i < sortedAttempts.size(); ++i) {
            const auto& attempt = *sortedAttempts[i];
            const std::string& answer = attempt.answers[q];
            
            // Severely penalize answers that resulted in very low scores
            double scoreWeight = attempt.percentage < 20.0 ? -0.5 : 1.0;
            double weight = std::exp(-0.1 * i) * scoreWeight;
            
            weightedScores[answer] += attempt.percentage * weight;
            totalWeights[answer] += std::abs(weight); // Use absolute value for total
        }
        
        // Find best answer considering penalties
        std::string bestAnswer;
        double bestConfidence = -1.0;
        
        for (const auto& [answer, scores] : answerScores) {
            // Skip answers that only appeared in very low scoring attempts
            if (std::all_of(scores.begin(), scores.end(), 
                           [](double score) { return score < 20.0; })) {
                continue;
            }
            
            // Calculate weighted average score
            double avgScore = weightedScores[answer] / totalWeights[answer];
            
            // Calculate score consistency
            double variance = 0.0;
            for (double score : scores) {
                variance += (score - avgScore) * (score - avgScore);
            }
            variance = scores.size() > 1 ? variance / (scores.size() - 1) : 0.0;
            
            // Calculate success rate in high-scoring attempts (top 50%)
            int highScoreSuccesses = 0;
            int highScoreTotal = 0;
            for (size_t i = 0; i < sortedAttempts.size() / 2 + 1; ++i) {
                if (sortedAttempts[i]->answers[q] == answer) {
                    if (sortedAttempts[i]->percentage >= 40.0) {
                        highScoreSuccesses++;
                    }
                }
                highScoreTotal++;
            }
            double highScoreRate = static_cast<double>(highScoreSuccesses) / highScoreTotal;
            
            // Combined confidence metric with stronger penalty for low scores
            double confidence = avgScore * 0.3 + highScoreRate * 100 * 0.7;
            confidence /= (1.0 + std::sqrt(variance) * 0.2);
            
            // Additional penalty for answers that appeared in very low scoring attempts
            for (double score : scores) {
                if (score < 20.0) {
                    confidence *= 0.5; // Reduce confidence by half for each very low score
                }
            }
            
            if (confidence > bestConfidence) {
                bestConfidence = confidence;
                bestAnswer = answer;
            }
        }
        
        // Normalize confidence and ensure very low scoring answers get very low confidence
        double normalizedConfidence = std::max(0.0, std::min(100.0, bestConfidence));
        result.emplace_back(bestAnswer, normalizedConfidence);
    }
    
    return result;
}

std::map<size_t, std::vector<std::string>> AnswerAnalyzer::getAnswerPatterns() const {
    std::map<size_t, std::vector<std::string>> patterns;
    
    for (const auto& attempt : attempts) {
        size_t scoreKey = static_cast<size_t>(std::round(attempt.percentage));
        patterns[scoreKey] = attempt.answers;
    }
    
    return patterns;
}

std::vector<std::string> AnswerAnalyzer::suggestNextAttempt() const {
    auto confidences = getAnswerConfidences();
    std::vector<std::string> suggestion;
    
    for (const auto& [answer, confidence] : confidences) {
        suggestion.push_back(answer);
    }
    
    return suggestion;
}


double AnswerAnalyzer::predictScore(const std::vector<std::string>& answers) const {
    if (attempts.empty() || answers.empty()) {
        return 0.0;
    }
    
    // Sort attempts by score
    std::vector<const TestAttempt*> sortedAttempts;
    for (const auto& attempt : attempts) {
        sortedAttempts.push_back(&attempt);
    }
    std::sort(sortedAttempts.begin(), sortedAttempts.end(),
        [](const TestAttempt* a, const TestAttempt* b) {
            return a->percentage > b->percentage;
        });
    
    // Calculate similarity scores with emphasis on matching high-scoring patterns
    std::vector<double> similarityScores;
    for (const auto& attempt : attempts) {
        double matchingScore = 0.0;
        double totalWeight = 0.0;
        
        for (size_t i = 0; i < answers.size() && i < attempt.answers.size(); ++i) {
            // Weight each question based on its confidence
            double confidence = getAnswerConfidences()[i].second / 100.0;
            double weight = 1.0 + confidence; // Questions with higher confidence count more
            
            if (answers[i] == attempt.answers[i]) {
                matchingScore += weight;
            }
            totalWeight += weight;
        }
        
        double similarity = totalWeight > 0.0 ? matchingScore / totalWeight : 0.0;
        similarityScores.push_back(similarity);
    }
    
    // Predict score using weighted average of similar attempts
    double totalWeight = 0.0;
    double weightedSum = 0.0;
    
    for (size_t i = 0; i < attempts.size(); ++i) {
        // Weight calculation considers both similarity and the attempt's score
        double weight = similarityScores[i] * similarityScores[i] * 
                       (1.0 + attempts[i].percentage / 100.0); // Boost weight for high-scoring attempts
        totalWeight += weight;
        weightedSum += weight * attempts[i].percentage;
    }
    
    return totalWeight > 0.0 ? weightedSum / totalWeight : 0.0;
}


double AnswerAnalyzer::getAverageScore() const {
    if (attempts.empty()) {
        return 0.0;
    }
    
    double sum = std::accumulate(attempts.begin(), attempts.end(), 0.0,
        [](double acc, const TestAttempt& attempt) {
            return acc + attempt.percentage;
        });
    
    return sum / attempts.size();
}

double AnswerAnalyzer::getScoreVariance() const {
    if (attempts.empty()) {
        return 0.0;
    }
    
    double mean = getAverageScore();
    double sumSquares = std::accumulate(attempts.begin(), attempts.end(), 0.0,
        [mean](double acc, const TestAttempt& attempt) {
            double diff = attempt.percentage - mean;
            return acc + (diff * diff);
        });
    
    return sumSquares / attempts.size();
}

void AnswerAnalyzer::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file) {
        throw AnswerAnalyzerException("Cannot open file for writing: " + filename);
    }
    
    try {
        // Save number of attempts
        file << attempts.size() << "\n";
        
        // Save each attempt
        for (const auto& attempt : attempts) {
            // Save number of answers
            file << attempt.answers.size() << "\n";
            
            // Save answers
            for (const auto& answer : attempt.answers) {
                file << answer << "\n";
            }
            
            // Save percentage
            file << attempt.percentage << "\n";
        }
        
        if (!file) {
            throw AnswerAnalyzerException("Error writing to file: " + filename);
        }
    }
    catch (...) {
        file.close();
        throw;
    }
    
    file.close();
}

void AnswerAnalyzer::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw AnswerAnalyzerException("Cannot open file for reading: " + filename);
    }
    
    try {
        clear();
        
        // Read number of attempts
        size_t numAttempts;
        file >> numAttempts;
        file.ignore(); // Skip newline
        
        // Read each attempt
        for (size_t i = 0; i < numAttempts; ++i) {
            // Read number of answers
            size_t numAnswers;
            file >> numAnswers;
            file.ignore(); // Skip newline
            
            std::vector<std::string> answers;
            
            // Read answers
            for (size_t j = 0; j < numAnswers; ++j) {
                std::string answer;
                std::getline(file, answer);
                answers.push_back(answer);
            }
            
            // Read percentage
            double percentage;
            file >> percentage;
            file.ignore(); // Skip newline
            
            addAttempt(answers, percentage);
        }
        
        if (file.fail() && !file.eof()) {
            throw AnswerAnalyzerException("Error reading from file: " + filename);
        }
    }
    catch (...) {
        file.close();
        throw;
    }
    
    file.close();
}