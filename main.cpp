#include "answerTracker.h"
#include "answerAnalyzer.h"
#include <iostream>
#include <limits>
#include <iomanip>

// Platform-specific console color support
#ifdef _WIN32
#include <windows.h>
void setTextColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}
#else
void setTextColor(int) {} // Dummy implementation for non-Windows
#endif

void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void displayMainMenu() {
    std::cout << "\n=== Answer Analysis System ===" << std::endl;
    std::cout << "1. Enter New Test Attempt" << std::endl;
    std::cout << "2. Analyze Results" << std::endl;
    std::cout << "3. View Statistics" << std::endl;
    std::cout << "4. Save/Load Data" << std::endl;
    std::cout << "5. View Help" << std::endl;
    std::cout << "6. Clear All Data" << std::endl;
    std::cout << "7. Exit" << std::endl;
    std::cout << "\nEnter your choice (1-7): ";
}

void displayStatisticsMenu() {
    std::cout << "\n=== Statistics Menu ===" << std::endl;
    std::cout << "1. View Basic Statistics" << std::endl;
    std::cout << "2. View Answer Confidences" << std::endl;
    std::cout << "3. View Answer Patterns" << std::endl;
    std::cout << "4. Get Next Attempt Suggestion" << std::endl;
    std::cout << "5. Return to Main Menu" << std::endl;
    std::cout << "\nEnter your choice (1-5): ";
}

void displayFileMenu() {
    std::cout << "\n=== File Operations ===" << std::endl;
    std::cout << "1. Save Analysis Data" << std::endl;
    std::cout << "2. Load Analysis Data" << std::endl;
    std::cout << "3. Return to Main Menu" << std::endl;
    std::cout << "\nEnter your choice (1-3): ";
}

void displayHelp() {
    std::cout << "\n=== Help Information ===" << std::endl;
    std::cout << "This system helps analyze multiple test attempts to determine correct answers.\n" << std::endl;
    std::cout << "Key Features:" << std::endl;
    std::cout << "- Enter multiple test attempts with answers and scores" << std::endl;
    std::cout << "- Analyze patterns to identify correct answers" << std::endl;
    std::cout << "- Get suggestions for your next attempt" << std::endl;
    std::cout << "- Track your progress over time" << std::endl;
    std::cout << "- Save and load your analysis data" << std::endl;
    
    std::cout << "\nTips for Better Results:" << std::endl;
    std::cout << "1. Try different answers in each attempt" << std::endl;
    std::cout << "2. Enter both high and low scoring attempts" << std::endl;
    std::cout << "3. Be consistent with your answer format" << std::endl;
    std::cout << "4. Save your data regularly" << std::endl;
    
    std::cout << "\nPress Enter to continue...";
    std::cin.get();
}

void enterNewAttempt(AnswerAnalyzer& analyzer) {
    std::vector<std::string> answers;
    double percentage;
    size_t numQuestions = analyzer.getFirstAttemptSize();
    
    if (numQuestions == 0) {
        std::cout << "Enter number of questions (1-" << analyzer.getMaxAnswers() << "): ";
        std::cin >> numQuestions;
        clearInputBuffer();
        
        if (numQuestions < 1 || numQuestions > analyzer.getMaxAnswers()) {
            throw AnswerAnalyzerException("Invalid number of questions");
        }
    }
    
    std::cout << "\nEntering new test attempt (" << numQuestions << " questions)\n";
    
    // Get suggested answers and confidences
    auto suggestedAnswers = analyzer.getMostCommonAnswers();
    auto confidences = analyzer.getAnswerConfidences();
    
    for (size_t i = 0; i < numQuestions; i++) {
        std::string suggested = (i < suggestedAnswers.size()) ? suggestedAnswers[i] : "";
        double confidence = (i < confidences.size()) ? confidences[i].second : 0.0;
        
        std::cout << "\nQuestion " << (i + 1) << ":";
        if (!suggested.empty()) {
            setTextColor(2); // Green
            std::cout << " (Suggested: " << suggested;
            if (confidence > 0.0) {
                std::cout << ", Confidence: " << std::fixed << std::setprecision(1) 
                         << confidence << "%";
            }
            std::cout << ")";
            setTextColor(7); // Reset color
        }
        std::cout << "\nYour answer: ";
        
        std::string answer;
        std::getline(std::cin, answer);
        
        if (answer.empty() && !suggested.empty()) {
            answer = suggested;
            std::cout << "Using suggested answer: " << answer << std::endl;
        }
        
        answers.push_back(answer);
    }
    
    std::cout << "Enter your percentage score (0-100): ";
    if (!(std::cin >> percentage)) {
        throw AnswerAnalyzerException("Invalid score format");
    }
    clearInputBuffer();
    
    analyzer.addAttempt(answers, percentage);
    
    // Predict next attempt score
    if (!suggestedAnswers.empty()) {
        double predictedScore = analyzer.predictScore(suggestedAnswers);
        std::cout << "\nPredicted score for suggested answers: " 
                 << std::fixed << std::setprecision(1) << predictedScore << "%" << std::endl;
    }
}

void viewStatistics(const AnswerAnalyzer& analyzer) {
    int choice;
    do {
        displayStatisticsMenu();
        std::cin >> choice;
        clearInputBuffer();
        
        switch (choice) {
            case 1: {
                std::cout << "\n=== Basic Statistics ===" << std::endl;
                std::cout << "Total Attempts: " << analyzer.getNumAttempts() << std::endl;
                std::cout << "Average Score: " << std::fixed << std::setprecision(1) 
                         << analyzer.getAverageScore() << "%" << std::endl;
                std::cout << "Score Variance: " << analyzer.getScoreVariance() << std::endl;
                break;
            }
                
            case 2: {
                auto confidences = analyzer.getAnswerConfidences();
                std::cout << "\n=== Answer Confidences ===" << std::endl;
                for (size_t i = 0; i < confidences.size(); ++i) {
                    std::cout << "Question " << (i + 1) << ": " 
                             << confidences[i].first << " (" 
                             << std::fixed << std::setprecision(1) 
                             << confidences[i].second << "% confidence)" << std::endl;
                }
                break;
            }
                
            case 3: {
                auto patterns = analyzer.getAnswerPatterns();
                std::cout << "\n=== Answer Patterns ===" << std::endl;
                for (const auto& [score, answers] : patterns) {
                    std::cout << "Score " << score << "%:" << std::endl;
                    for (const auto& answer : answers) {
                        std::cout << "  " << answer << std::endl;
                    }
                }
                break;
            }
                
            case 4: {
                auto suggestion = analyzer.suggestNextAttempt();
                if (!suggestion.empty()) {
                    std::cout << "\n=== Suggested Answers for Next Attempt ===" << std::endl;
                    for (size_t i = 0; i < suggestion.size(); ++i) {
                        std::cout << "Question " << (i + 1) << ": " 
                                 << suggestion[i] << std::endl;
                    }
                    double predictedScore = analyzer.predictScore(suggestion);
                    std::cout << "\nPredicted score: " << std::fixed << std::setprecision(1) 
                             << predictedScore << "%" << std::endl;
                } else {
                    std::cout << "Not enough data for suggestions." << std::endl;
                }
                break;
            }
                
            case 5:
                return;
                
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }
        
        std::cout << "\nPress Enter to continue...";
        std::cin.get();
        
    } while (true);
}

void handleFileOperations(AnswerAnalyzer& analyzer) {
    int choice;
    std::string filename;
    
    do {
        displayFileMenu();
        std::cin >> choice;
        clearInputBuffer();
        
        switch (choice) {
            case 1: {
                std::cout << "Enter filename to save: ";
                std::getline(std::cin, filename);
                analyzer.saveToFile(filename);
                std::cout << "Data saved successfully!" << std::endl;
                break;
            }
                
            case 2: {
                std::cout << "Enter filename to load: ";
                std::getline(std::cin, filename);
                analyzer.loadFromFile(filename);
                std::cout << "Data loaded successfully!" << std::endl;
                break;
            }
                
            case 3:
                return;
                
            default:
                std::cout << "Invalid choice. Please try again." << std::endl;
        }
        
    } while (true);
}

int main() {
    AnswerAnalyzer analyzer;
    int choice;
    
    std::cout << "Welcome to the Answer Analysis System!" << std::endl;
    displayHelp();
    
    while (true) {
        try {
            displayMainMenu();
            std::cin >> choice;
            clearInputBuffer();
            
            switch (choice) {
                case 1:
                    enterNewAttempt(analyzer);
                    break;
                    
                case 2:
                    analyzer.analyzeResults();
                    break;
                    
                case 3:
                    viewStatistics(analyzer);
                    break;
                    
                case 4:
                    handleFileOperations(analyzer);
                    break;
                    
                case 5:
                    displayHelp();
                    break;
                    
                case 6: {
                    if (analyzer.getNumAttempts() > 0) {
                        char confirm;
                        std::cout << "Are you sure you want to clear all data? (y/n): ";
                        std::cin >> confirm;
                        clearInputBuffer();
                        
                        if (tolower(confirm) == 'y') {
                            analyzer.clear();
                            std::cout << "All data cleared!" << std::endl;
                        }
                    } else {
                        std::cout << "No data to clear." << std::endl;
                    }
                    break;
                }
                    
                case 7: {
                    if (analyzer.getNumAttempts() > 0) {
                        char confirm;
                        std::cout << "You have unsaved data. Are you sure you want to exit? (y/n): ";
                        std::cin >> confirm;
                        clearInputBuffer();
                        
                        if (tolower(confirm) != 'y') {
                            break;
                        }
                    }
                    std::cout << "Thank you for using the Answer Analysis System. Goodbye!" << std::endl;
                    return 0;
                }
                    
                default:
                    std::cout << "Invalid choice. Please try again." << std::endl;
            }
            
        } catch (const std::exception& e) {
            std::cout << "Error: " << e.what() << std::endl;
        }
    }
    
    return 0;
}