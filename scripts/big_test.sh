#!/bin/bash

# ============================================================================
# Script de test massif pour lem-in
# Génère 10x chaque style de map et teste toutes les maps générées
# ============================================================================

# Couleurs pour l'affichage
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
CYAN='\033[0;36m'
RESET='\033[0m'
BOLD='\033[1m'

# Chemins
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

# Styles de maps à générer
MAP_STYLES=(
    "flow-one"
    "flow-ten"
    "flow-thousand"
    "big"
    "big-superposition"
)

# Compteurs
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
SLOW_TESTS=0
TIMEOUT_TESTS=0

# Dossier de logs pour les échecs
LOG_DIR="$PROJECT_DIR/resources/big_test_logs"
LOG_FILE=""

# Prépare le fichier de log à la première utilisation
prepare_log_file() {
    if [ -z "$LOG_FILE" ]; then
        mkdir -p "$LOG_DIR"
        local timestamp
        timestamp=$(date +%Y%m%d_%H%M%S)
        LOG_FILE="$LOG_DIR/big_test_${timestamp}.log"
        touch "$LOG_FILE"
    fi
}

# Ajoute une entrée au log et sauvegarde la map échouée
log_failure() {
    local message="$1"
    local map_file="$2"
    local map_name="$3"

    prepare_log_file
    echo "$message" >> "$LOG_FILE"

    if [ -f "$map_file" ]; then
        local dest_file="$LOG_DIR/${map_name}.txt"
        if [ -e "$dest_file" ]; then
            local counter=1
            while [ -e "${LOG_DIR}/${map_name}_${counter}.txt" ]; do
                counter=$((counter + 1))
            done
            dest_file="${LOG_DIR}/${map_name}_${counter}.txt"
        fi
        cp "$map_file" "$dest_file"
    fi
}

# Fonction pour afficher un message d'erreur
error() {
    echo -e "${RED}${BOLD}[ERROR]${RESET} $1" >&2
}

# Fonction pour afficher un message de succès
success() {
    echo -e "${GREEN}${BOLD}[SUCCESS]${RESET} $1"
}

# Fonction pour afficher un message d'info
info() {
    echo -e "${BLUE}${BOLD}[INFO]${RESET} $1"
}

# Fonction pour afficher un message de warning
warning() {
    echo -e "${YELLOW}${BOLD}[WARNING]${RESET} $1"
}

# Détecter le générateur selon l'OS
detect_generator() {
    local os_name
    os_name=$(uname -s 2>/dev/null)

    case "$os_name" in
        Darwin*)
            echo "$PROJECT_DIR/resources/map_generator/generator_osx"
            ;;
        Linux*)
            echo "$PROJECT_DIR/resources/map_generator/generator_linux"
            ;;
        *)
            error "OS non supporté: $os_name"
            exit 1
            ;;
    esac
}

GENERATOR=$(detect_generator)

if [ ! -f "$GENERATOR" ]; then
    error "Générateur non trouvé pour $(uname -s): $GENERATOR"
    exit 1
fi

LEMIN="$PROJECT_DIR/lem-in"
TEST_DIR="$PROJECT_DIR/resources/big_test_maps"

# Vérifier que les fichiers nécessaires existent
if [ ! -f "$GENERATOR" ]; then
    error "Générateur non trouvé: $GENERATOR"
    exit 1
fi

if [ ! -f "$LEMIN" ]; then
    error "lem-in non trouvé: $LEMIN"
    error "Compile d'abord avec: make"
    exit 1
fi

# Créer le dossier de test
mkdir -p "$TEST_DIR"
info "Dossier de test: $TEST_DIR"

# Fonction pour générer une map
generate_map() {
    local style=$1
    local index=$2
    local output_file="$TEST_DIR/${style}_${index}"
    
    if [ "$style" = "flow-one" ]; then
        "$GENERATOR" --flow-one > "$output_file" 2>/dev/null
    elif [ "$style" = "flow-ten" ]; then
        "$GENERATOR" --flow-ten > "$output_file" 2>/dev/null
    elif [ "$style" = "flow-thousand" ]; then
        "$GENERATOR" --flow-thousand > "$output_file" 2>/dev/null
    elif [ "$style" = "big" ]; then
        "$GENERATOR" --big > "$output_file" 2>/dev/null
    elif [ "$style" = "big-superposition" ]; then
        "$GENERATOR" --big-superposition > "$output_file" 2>/dev/null
    else
        error "Style inconnu: $style"
        return 1
    fi
    
    if [ $? -ne 0 ] || [ ! -f "$output_file" ] || [ ! -s "$output_file" ]; then
        error "Échec de génération: ${style}_${index}"
        return 1
    fi
    
    return 0
}

# Fonction pour tester une map
test_map() {
    local map_file=$1
    local map_name=$(basename "$map_file")
    
    # Extraire le nombre requis
    local required=$(grep -m1 -E '^#Here is the number of lines required: *[0-9]+' "$map_file" 2>/dev/null | sed -E 's/.*: *([0-9]+).*/\1/')
    
    if [ -z "$required" ]; then
        local plain_msg="FAIL    ${map_name:0:50} -> missing required-lines header"
        echo -e "${RED}FAIL${RESET}    ${map_name:0:50} -> ${RED}missing required-lines header${RESET}"
        log_failure "$plain_msg" "$map_file" "$map_name"
        return 1
    fi
    
    # Mesurer le temps d'exécution avec la commande time
    # TIMEFORMAT='%R' donne le temps réel en secondes (format décimal)
    # time écrit sur stderr, donc on capture stderr séparément
    local TIMEFORMAT='%R'
    local temp_output=$(mktemp)
    local time_stderr=$( { time "$LEMIN" < "$map_file" 2>/dev/null > "$temp_output"; } 2>&1 )
    local output=$(cat "$temp_output" 2>/dev/null)
    rm -f "$temp_output"
    
    # Extraire le temps réel (dernière ligne de stderr qui contient la sortie de time)
    local elapsed=$(echo "$time_stderr" | tail -1)
    
    # Vérifier que elapsed est un nombre valide
    if ! echo "$elapsed" | grep -qE '^[0-9]+\.?[0-9]*$'; then
        local plain_msg="FAIL    ${map_name:0:50} -> unable to measure execution time"
        error "Impossible de mesurer le temps pour ${map_name}"
        log_failure "$plain_msg" "$map_file" "$map_name"
        return 1
    fi
    
    # Convertir le temps décimal en entier (arrondi vers le haut pour être strict)
    # Utiliser awk pour arrondir vers le haut
    local elapsed_int=$(echo "$elapsed" | awk '{printf "%.0f", ($1 + 0.5)}')
    
    # Vérifier le timeout (15 secondes) - invalide la map si dépassé
    # Selon le sujet: "15 seconds is too much" → la map est invalide
    if [ "$elapsed_int" -ge 15 ]; then
        TIMEOUT_TESTS=$((TIMEOUT_TESTS + 1))
        local plain_msg="FAIL    ${map_name:0:50} -> TIMEOUT (${elapsed}s, max=15s) - MAP INVALIDE"
        echo -e "${RED}FAIL${RESET}    ${map_name:0:50} -> ${RED}TIMEOUT (${elapsed}s, max=15s) - MAP INVALIDE${RESET}"
        log_failure "$plain_msg" "$map_file" "$map_name"
        return 1
    fi
    
    local got=$(echo "$output" | awk -F': ' '/^# Number of lines: /{v=$2} END{if (v) print v}')
    
    if [ -z "$got" ]; then
        local plain_msg="FAIL    ${map_name:0:50} -> no result found (obtenu N/A / voulu $required, temps=${elapsed}s)"
        echo -e "${RED}FAIL${RESET}    ${map_name:0:50} -> ${RED}no result found (obtenu N/A / voulu $required, temps=${elapsed}s)${RESET}"
        log_failure "$plain_msg" "$map_file" "$map_name"
        return 1
    fi
    
    # Déterminer le statut du temps (utiliser elapsed_int pour les comparaisons)
    local time_status=""
    local time_color=""
    if [ "$elapsed_int" -le 3 ]; then
        time_status="excellent"
        time_color="${GREEN}"
    elif [ "$elapsed_int" -le 9 ]; then
        time_status="mediocre"
        time_color="${YELLOW}"
        SLOW_TESTS=$((SLOW_TESTS + 1))
    else
        time_status="slow"
        time_color="${RED}"
        SLOW_TESTS=$((SLOW_TESTS + 1))
    fi
    
    # Déterminer le statut selon le nombre de tours
    local status_label=""
    local status_color=""
    if [ "$got" -le "$required" ]; then
        status_label="PERFECT"
        status_color="${CYAN}"
    elif [ "$got" -gt $((required + 10)) ]; then
        status_label="WARNING"
        status_color="${YELLOW}"
    else
        status_label="SUCCESS"
        status_color="${GREEN}"
    fi

    # Afficher le résultat
    local result_text="obtenu/voulu: ${got}/${required}"
    if [ "$elapsed_int" -le 9 ]; then
        echo -e "${status_color}${status_label}${RESET} ${map_name:0:50} -> ${result_text}, time=${time_color}${elapsed}s${RESET} (${time_status})"
    else
        echo -e "${status_color}${status_label}${RESET} ${map_name:0:50} -> ${result_text}, time=${time_color}${elapsed}s${RESET} (${time_status}) ${YELLOW}⚠${RESET}"
    fi

    # WARNING reste un succès logique (non optimal) -> pas d'échec bloquant
    if [ "$status_label" = "WARNING" ]; then
        warning "${map_name:0:50} -> plus de 10 lignes au-dessus du requis (obtenu=$got / voulu=$required)"
    fi

    return 0
}

# Générer toutes les maps
info "Génération de 10 maps pour chaque style..."
for style in "${MAP_STYLES[@]}"; do
    info "Génération des maps: $style"
    for i in {1..10}; do
        if generate_map "$style" "$i"; then
            echo -e "  ${CYAN}✓${RESET} ${style}_${i} générée"
            sleep 1
        else
            echo -e "  ${RED}✗${RESET} ${style}_${i} échec"
            sleep 1
        fi
    done
done

echo ""
info "Début des tests..."

# Tester toutes les maps générées
for map_file in "$TEST_DIR"/*; do
    if [ -f "$map_file" ]; then
        TOTAL_TESTS=$((TOTAL_TESTS + 1))
        if test_map "$map_file"; then
            PASSED_TESTS=$((PASSED_TESTS + 1))
        else
            FAILED_TESTS=$((FAILED_TESTS + 1))
        fi
    fi
done

# Résumé final
echo ""
echo "=========================================="
if [ $FAILED_TESTS -eq 0 ] && [ $TIMEOUT_TESTS -eq 0 ]; then
    success "Tous les tests sont passés !"
    echo -e "${GREEN}${BOLD}Résultat:${RESET} $PASSED_TESTS/$TOTAL_TESTS tests réussis"
    if [ $SLOW_TESTS -gt 0 ]; then
        warning "$SLOW_TESTS test(s) avec performance médiocre (>3s) ou lente (>9s)"
    fi
    if [ -n "$LOG_FILE" ]; then
        warning "Voir les logs dans $LOG_FILE (maps copiées dans $LOG_DIR)"
    fi
else
    error "Certains tests ont échoué"
    echo -e "${RED}${BOLD}Échecs:${RESET} $FAILED_TESTS/$TOTAL_TESTS"
    echo -e "${GREEN}${BOLD}Réussis:${RESET} $PASSED_TESTS/$TOTAL_TESTS"
    if [ $TIMEOUT_TESTS -gt 0 ]; then
        echo -e "${RED}${BOLD}Timeouts:${RESET} $TIMEOUT_TESTS test(s) > 15s"
    fi
    if [ $SLOW_TESTS -gt 0 ]; then
        warning "$SLOW_TESTS test(s) avec performance médiocre (>3s) ou lente (>9s)"
    fi
    if [ -n "$LOG_FILE" ]; then
        warning "Voir les logs dans $LOG_FILE (maps copiées dans $LOG_DIR)"
    fi
fi
echo "=========================================="
echo ""
echo -e "${CYAN}${BOLD}Critères de performance:${RESET}"
echo -e "  ${GREEN}≤ 3s${RESET}  : Excellent (2-3s est idéal)"
echo -e "  ${YELLOW}4-9s${RESET}  : Médiocre (9s est acceptable mais pas optimal)"
echo -e "  ${RED}10-14s${RESET} : Lent (acceptable mais à optimiser)"
echo -e "  ${RED}≥ 15s${RESET}  : ${RED}${BOLD}Trop lent - MAP INVALIDE${RESET} (selon le sujet: '15 seconds is too much')"

# Nettoyer les maps générées
echo ""
info "Nettoyage des maps générées..."
if [ -d "$TEST_DIR" ]; then
    rm -rf "$TEST_DIR"
    if [ $? -eq 0 ]; then
        echo -e "  ${CYAN}✓${RESET} Dossier $TEST_DIR supprimé"
    else
        warning "Impossible de supprimer $TEST_DIR"
    fi
else
    echo -e "  ${CYAN}✓${RESET} Aucun dossier à nettoyer"
fi

# Retourner le code d'erreur approprié
if [ $FAILED_TESTS -eq 0 ] && [ $TIMEOUT_TESTS -eq 0 ]; then
    exit 0
else
    exit 1
fi

