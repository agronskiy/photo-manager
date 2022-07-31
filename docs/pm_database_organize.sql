--
-- PostgreSQL database dump
--

-- Started on 2009-01-30 17:54:18

SET client_encoding = 'UTF8';
SET standard_conforming_strings = off;
SET check_function_bodies = false;
SET client_min_messages = warning;
SET escape_string_warning = off;

SET search_path = public, pg_catalog;

SET default_tablespace = '';

SET default_with_oids = false;

--
-- TOC entry 1505 (class 1259 OID 16868)
-- Dependencies: 5
-- Name: pm_aux; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE pm_aux (
    name character varying,
    value character varying
);


--
-- TOC entry 1495 (class 1259 OID 16708)
-- Dependencies: 5
-- Name: pm_bigimages; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE pm_bigimages (
    id bigint NOT NULL,
    image bytea
);


--
-- TOC entry 1496 (class 1259 OID 16714)
-- Dependencies: 5
-- Name: pm_categories; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE pm_categories (
    id bigint NOT NULL,
    name text,
    pid bigint,
    amount bigint,
    outdated "char"
);


--
-- TOC entry 1497 (class 1259 OID 16720)
-- Dependencies: 1773 5
-- Name: pm_images; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE pm_images (
    id bigint NOT NULL,
    name text,
    location text,
    header text,
    description text,
    pid character varying,
    date date,
    preview bytea,
    inventary bigint DEFAULT 0,
    author character varying,
    cd character varying,
    width integer,
    height integer,
    size real
);


--
-- TOC entry 1498 (class 1259 OID 16727)
-- Dependencies: 5
-- Name: pm_inv; Type: TABLE; Schema: public; Owner: -; Tablespace: 
--

CREATE TABLE pm_inv (
    id bigint NOT NULL,
    name character varying(256),
    author character varying(256),
    header text,
    coloured character(1),
    internal_usage character(1),
    type character varying(10),
    date date,
    comments text
);


--
-- TOC entry 312 (class 1247 OID 16735)
-- Dependencies: 5 1499
-- Name: tablefunc_crosstab_2; Type: TYPE; Schema: public; Owner: -
--

CREATE TYPE tablefunc_crosstab_2 AS (
	row_name text,
	category_1 text,
	category_2 text
);


--
-- TOC entry 314 (class 1247 OID 16738)
-- Dependencies: 5 1500
-- Name: tablefunc_crosstab_3; Type: TYPE; Schema: public; Owner: -
--

CREATE TYPE tablefunc_crosstab_3 AS (
	row_name text,
	category_1 text,
	category_2 text,
	category_3 text
);


--
-- TOC entry 316 (class 1247 OID 16741)
-- Dependencies: 5 1501
-- Name: tablefunc_crosstab_4; Type: TYPE; Schema: public; Owner: -
--

CREATE TYPE tablefunc_crosstab_4 AS (
	row_name text,
	category_1 text,
	category_2 text,
	category_3 text,
	category_4 text
);


--
-- TOC entry 20 (class 1255 OID 16742)
-- Dependencies: 5
-- Name: connectby(text, text, text, text, integer, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION connectby(text, text, text, text, integer, text) RETURNS SETOF record
    AS '$libdir/tablefunc', 'connectby_text'
    LANGUAGE c STABLE STRICT;


--
-- TOC entry 21 (class 1255 OID 16743)
-- Dependencies: 5
-- Name: connectby(text, text, text, text, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION connectby(text, text, text, text, integer) RETURNS SETOF record
    AS '$libdir/tablefunc', 'connectby_text'
    LANGUAGE c STABLE STRICT;


--
-- TOC entry 22 (class 1255 OID 16744)
-- Dependencies: 5
-- Name: connectby(text, text, text, text, text, integer, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION connectby(text, text, text, text, text, integer, text) RETURNS SETOF record
    AS '$libdir/tablefunc', 'connectby_text_serial'
    LANGUAGE c STABLE STRICT;


--
-- TOC entry 23 (class 1255 OID 16745)
-- Dependencies: 5
-- Name: connectby(text, text, text, text, text, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION connectby(text, text, text, text, text, integer) RETURNS SETOF record
    AS '$libdir/tablefunc', 'connectby_text_serial'
    LANGUAGE c STABLE STRICT;


--
-- TOC entry 24 (class 1255 OID 16746)
-- Dependencies: 5
-- Name: crosstab(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION crosstab(text) RETURNS SETOF record
    AS '$libdir/tablefunc', 'crosstab'
    LANGUAGE c STABLE STRICT;


--
-- TOC entry 25 (class 1255 OID 16747)
-- Dependencies: 5
-- Name: crosstab(text, integer); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION crosstab(text, integer) RETURNS SETOF record
    AS '$libdir/tablefunc', 'crosstab'
    LANGUAGE c STABLE STRICT;


--
-- TOC entry 26 (class 1255 OID 16748)
-- Dependencies: 5
-- Name: crosstab(text, text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION crosstab(text, text) RETURNS SETOF record
    AS '$libdir/tablefunc', 'crosstab_hash'
    LANGUAGE c STABLE STRICT;


--
-- TOC entry 27 (class 1255 OID 16749)
-- Dependencies: 5 312
-- Name: crosstab2(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION crosstab2(text) RETURNS SETOF tablefunc_crosstab_2
    AS '$libdir/tablefunc', 'crosstab'
    LANGUAGE c STABLE STRICT;


--
-- TOC entry 28 (class 1255 OID 16750)
-- Dependencies: 5 314
-- Name: crosstab3(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION crosstab3(text) RETURNS SETOF tablefunc_crosstab_3
    AS '$libdir/tablefunc', 'crosstab'
    LANGUAGE c STABLE STRICT;


--
-- TOC entry 29 (class 1255 OID 16751)
-- Dependencies: 316 5
-- Name: crosstab4(text); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION crosstab4(text) RETURNS SETOF tablefunc_crosstab_4
    AS '$libdir/tablefunc', 'crosstab'
    LANGUAGE c STABLE STRICT;


--
-- TOC entry 30 (class 1255 OID 16752)
-- Dependencies: 5
-- Name: normal_rand(integer, double precision, double precision); Type: FUNCTION; Schema: public; Owner: -
--

CREATE FUNCTION normal_rand(integer, double precision, double precision) RETURNS SETOF double precision
    AS '$libdir/tablefunc', 'normal_rand'
    LANGUAGE c STRICT;


--
-- TOC entry 1502 (class 1259 OID 16753)
-- Dependencies: 1496 5
-- Name: pm_categories_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE pm_categories_id_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- TOC entry 1795 (class 0 OID 0)
-- Dependencies: 1502
-- Name: pm_categories_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE pm_categories_id_seq OWNED BY pm_categories.id;


--
-- TOC entry 1503 (class 1259 OID 16755)
-- Dependencies: 1497 5
-- Name: pm_images_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE pm_images_id_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- TOC entry 1796 (class 0 OID 0)
-- Dependencies: 1503
-- Name: pm_images_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE pm_images_id_seq OWNED BY pm_images.id;


--
-- TOC entry 1504 (class 1259 OID 16757)
-- Dependencies: 5 1498
-- Name: pm_inv_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE pm_inv_id_seq
    INCREMENT BY 1
    NO MAXVALUE
    NO MINVALUE
    CACHE 1;


--
-- TOC entry 1797 (class 0 OID 0)
-- Dependencies: 1504
-- Name: pm_inv_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE pm_inv_id_seq OWNED BY pm_inv.id;


--
-- TOC entry 1772 (class 2604 OID 16759)
-- Dependencies: 1502 1496
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE pm_categories ALTER COLUMN id SET DEFAULT nextval('pm_categories_id_seq'::regclass);


--
-- TOC entry 1774 (class 2604 OID 16760)
-- Dependencies: 1503 1497
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE pm_images ALTER COLUMN id SET DEFAULT nextval('pm_images_id_seq'::regclass);


--
-- TOC entry 1775 (class 2604 OID 16761)
-- Dependencies: 1504 1498
-- Name: id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE pm_inv ALTER COLUMN id SET DEFAULT nextval('pm_inv_id_seq'::regclass);


--
-- TOC entry 1780 (class 2606 OID 16763)
-- Dependencies: 1497 1497
-- Name: pki_pm_images; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY pm_images
    ADD CONSTRAINT pki_pm_images PRIMARY KEY (id);


--
-- TOC entry 1777 (class 2606 OID 16765)
-- Dependencies: 1495 1495
-- Name: uni_bigimages; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY pm_bigimages
    ADD CONSTRAINT uni_bigimages UNIQUE (id);


--
-- TOC entry 1782 (class 2606 OID 16767)
-- Dependencies: 1498 1498
-- Name: uni_pm_images; Type: CONSTRAINT; Schema: public; Owner: -; Tablespace: 
--

ALTER TABLE ONLY pm_inv
    ADD CONSTRAINT uni_pm_images UNIQUE (id);


--
-- TOC entry 1778 (class 1259 OID 16768)
-- Dependencies: 1497
-- Name: fki_pm_to_pm_inv; Type: INDEX; Schema: public; Owner: -; Tablespace: 
--

CREATE INDEX fki_pm_to_pm_inv ON pm_images USING btree (inventary);


--
-- TOC entry 1783 (class 2606 OID 16769)
-- Dependencies: 1495 1497 1779
-- Name: to_images; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY pm_bigimages
    ADD CONSTRAINT to_images FOREIGN KEY (id) REFERENCES pm_images(id) ON DELETE CASCADE;


--
-- TOC entry 1784 (class 2606 OID 16774)
-- Dependencies: 1497 1498 1781
-- Name: to_inv; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY pm_images
    ADD CONSTRAINT to_inv FOREIGN KEY (inventary) REFERENCES pm_inv(id) ON DELETE CASCADE;




--
-- PostgreSQL database dump complete
--

