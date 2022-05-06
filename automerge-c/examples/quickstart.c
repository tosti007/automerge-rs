#include <stdio.h>
#include <stdlib.h>

#include <automerge.h>

AMvalue test(AMresult*, AMvalueVariant const);

/*
 *  Based on https://automerge.github.io/docs/quickstart
 */
int main(int argc, char** argv) {
    AMdoc* const doc1 = AMalloc();
    AMresult* const cards_result = AMmapPutObject(doc1, AM_ROOT, "cards", AM_OBJ_TYPE_LIST);
    AMvalue value = test(cards_result, AM_VALUE_OBJ_ID);
    AMobjId const* const cards = value.obj_id;
    AMresult* const card1_result = AMlistPutObject(doc1, cards, 0, true, AM_OBJ_TYPE_MAP);
    value = test(card1_result, AM_VALUE_OBJ_ID);
    AMobjId const* const card1 = value.obj_id;
    AMresult* result = AMmapPutStr(doc1, card1, "title", "Rewrite everything in Clojure");
    test(result, AM_VALUE_VOID);
    AMfreeResult(result);
    result = AMmapPutBool(doc1, card1, "done", false);
    test(result, AM_VALUE_VOID);
    AMfreeResult(result);
    AMresult* const card2_result = AMlistPutObject(doc1, cards, 0, true, AM_OBJ_TYPE_MAP);
    value = test(card2_result, AM_VALUE_OBJ_ID);
    AMobjId const* const card2 = value.obj_id;
    result = AMmapPutStr(doc1, card2, "title", "Rewrite everything in Haskell");
    test(result, AM_VALUE_VOID);
    AMfreeResult(result);
    result = AMmapPutBool(doc1, card2, "done", false);
    test(result, AM_VALUE_VOID);
    AMfreeResult(result);
    AMfreeResult(card2_result);
    result = AMcommit(doc1, "Add card", NULL);
    test(result, AM_VALUE_CHANGE_HASHES);
    AMfreeResult(result);

    AMdoc* doc2 = AMalloc();
    result = AMmerge(doc2, doc1);
    test(result, AM_VALUE_CHANGE_HASHES);
    AMfreeResult(result);
    AMfreeDoc(doc2);

    AMresult* const save_result = AMsave(doc1);
    value = test(save_result, AM_VALUE_BYTES);
    AMbyteSpan binary = value.bytes;
    doc2 = AMalloc();
    result = AMload(doc2, binary.src, binary.count);
    test(result, AM_VALUE_UINT);
    AMfreeResult(result);
    AMfreeResult(save_result);

    result = AMmapPutBool(doc1, card1, "done", true);
    test(result, AM_VALUE_VOID);
    AMfreeResult(result);
    result = AMcommit(doc1, "Mark card as done", NULL);
    test(result, AM_VALUE_CHANGE_HASHES);
    AMfreeResult(result);
    AMfreeResult(card1_result);

    result = AMlistDelete(doc2, cards, 0);
    test(result, AM_VALUE_VOID);
    AMfreeResult(result);
    result = AMcommit(doc2, "Delete card", NULL);
    test(result, AM_VALUE_CHANGE_HASHES);
    AMfreeResult(result);

    result = AMmerge(doc1, doc2);
    test(result, AM_VALUE_CHANGE_HASHES);
    AMfreeResult(result);
    AMfreeDoc(doc2);

    result = AMgetChanges(doc1, NULL);
    value = test(result, AM_VALUE_CHANGES);
    AMchange const* change = NULL;
    while (value.changes.ptr && (change = AMnextChange(&value.changes, 1))) {
        size_t const size = AMobjSizeAt(doc1, cards, change);
        printf("%s %ld\n", AMgetMessage(change), size);
    }
    AMfreeResult(result);
    AMfreeResult(cards_result);
    AMfreeDoc(doc1);
}

/**
 * \brief Extracts an `AMvalue` struct with discriminant \p value_tag
 *        from \p result or writes a message to `stderr`, frees \p result
 *        and terminates the program.
 *
.* \param[in] result A pointer to an `AMresult` struct.
 * \param[in] value_tag An `AMvalue` struct discriminant.
 * \return An `AMvalue` struct.
 * \pre \p result must be a valid address.
 */
AMvalue test(AMresult* result, AMvalueVariant const value_tag) {
    static char prelude[64];

    if (result == NULL) {
        fprintf(stderr, "NULL `AMresult` struct pointer.");
        exit(EXIT_FAILURE);
    }
    AMstatus const status = AMresultStatus(result);
    if (status != AM_STATUS_OK) {
        switch (status) {
            case AM_STATUS_ERROR:          sprintf(prelude, "Error");          break;
            case AM_STATUS_INVALID_RESULT: sprintf(prelude, "Invalid result"); break;
            default: sprintf(prelude, "Unknown `AMstatus` tag %d", status);
        }
        fprintf(stderr, "%s; %s.", prelude, AMerrorMessage(result));
        AMfreeResult(result);
        exit(EXIT_FAILURE);
    }
    AMvalue const value = AMresultValue(result, 0);
    if (value.tag != value_tag) {
        char const* label = NULL;
        switch (value.tag) {
            case AM_VALUE_ACTOR_ID:      label = "AM_VALUE_ACTOR_ID";      break;
            case AM_VALUE_BOOLEAN:       label = "AM_VALUE_BOOLEAN";       break;
            case AM_VALUE_BYTES:         label = "AM_VALUE_BYTES";         break;
            case AM_VALUE_CHANGE_HASHES: label = "AM_VALUE_CHANGE_HASHES"; break;
            case AM_VALUE_CHANGES:       label = "AM_VALUE_CHANGES";       break;
            case AM_VALUE_COUNTER:       label = "AM_VALUE_COUNTER";       break;
            case AM_VALUE_F64:           label = "AM_VALUE_F64";           break;
            case AM_VALUE_INT:           label = "AM_VALUE_INT";           break;
            case AM_VALUE_VOID:          label = "AM_VALUE_VOID";          break;
            case AM_VALUE_NULL:          label = "AM_VALUE_NULL";          break;
            case AM_VALUE_OBJ_ID:        label = "AM_VALUE_OBJ_ID";        break;
            case AM_VALUE_STR:           label = "AM_VALUE_STR";           break;
            case AM_VALUE_TIMESTAMP:     label = "AM_VALUE_TIMESTAMP";     break;
            case AM_VALUE_UINT:          label = "AM_VALUE_UINT";          break;
            default:                     label = "<unknown>";
        }
        fprintf(stderr, "Unexpected `AMvalueVariant` tag `%s` (%d).", label, value.tag);
        AMfreeResult(result);
        exit(EXIT_FAILURE);
    }
    return value;
}